#!/usr/bin/env python3
import os
import sys
import subprocess
import multiprocessing
from concurrent.futures import ProcessPoolExecutor
from tqdm import tqdm
import glob
import itertools
import json
import argparse
import time
import re
from datetime import timedelta
from threading import Lock

# Define paths
DATASET_DIR = "/home/kuchkr/Desktop/dataset"
OUTPUT_DIR = "/home/kuchkr/Desktop/original_mpeg7_files/out"
MPEG7_APP = "./lib/build/x64/Release/mpeg7_app"
NUM_CORES = multiprocessing.cpu_count()
MAX_JOBS = NUM_CORES 

# Counters for tracking errors
ERROR_COUNTER = 0
ERROR_COUNTER_LOCK = Lock()
ERROR_DETAILS = []
ERROR_DETAILS_LOCK = Lock()
# Real-time tracking of errors by descriptor
DESCRIPTOR_ERROR_COUNTS = {}
DESCRIPTOR_ERROR_LOCK = Lock()

# Define descriptors and their parameter combinations
DESCRIPTORS = {
    # Descriptor 1: Dominant Color Descriptor
    1: {
        "name": "DOMINANT_COLOR",
        "params": [
            {"VariancePresent": ["0", "1"], "SpatialCoherency": ["0", "1"]}
        ]
    },
    # Descriptor 2: Scalable Color Descriptor
    2: {
        "name": "SCALABLE_COLOR",
        "params": [
            {"NumberOfCoefficients": ["16", "32", "64", "128", "256"], 
             "NumberOfBitplanesDiscarded": ["0", "1", "2", "3", "4", "6", "8"] }
        ]
    },
    # Descriptor 3: Color Layout Descriptor
    3: {
        "name": "COLOR_LAYOUT",
        "params": [
            {"NumberOfYCoeff": ["3", "6", "10", "15", "21", "28", "64"],
                "NumberOfCCoeff": ["3", "6", "10", "15", "21", "28", "64"],
            }
        ]
    },
    # Descriptor 4: Color Structure Descriptor
    4: {
        "name": "COLOR_STRUCTURE",
        "params": [
            {"ColorQuantSize": ["32", "64", "128", "256"] }
        ]
    },
    # Descriptor 5: CT Browsing Descriptor
    5: {
        "name": "CT_BROWSING",
        "params": [
            {}  # No specific parameters
        ]
    },
    # Descriptor 6: Homogeneous Texture Descriptor
    6: {
        "name": "HOMOGENEOUS_TEXTURE",
        "params": [
            {"layer": ["0", "1"]}
        ]
    },
    # Descriptor 7: Texture Browsing Descriptor
    7: {
        "name": "TEXTURE_BROWSING",
        "params": [
            {
                "layer": ["0", "1"],
            }
        ]
    },
    # Descriptor 8: Edge Histogram Descriptor
    8: {
        "name": "EDGE_HISTOGRAM",
        "params": [
            {}  # No specific parameters
        ]
    },
    # Descriptor 9: Region Shape Descriptor
    9: {
        "name": "REGION_SHAPE",
        "params": [
            {}  # No specific parameters
        ]
    },
    # Descriptor 10: Contour Shape Descriptor
    10: {
        "name": "CONTOUR_SHAPE",
        "params": [
            {}  # No specific parameters
        ]
    }
}

def is_numeric_output(output_text):
    """Check if the descriptor output is just a number instead of XML"""
    # Skip the metadata line
    lines = output_text.strip().split('\n')
    if len(lines) >= 2 and lines[0].startswith('{') and lines[1] == "Descriptor XML:":
        # If there's only one line after "Descriptor XML:" and it's numeric
        if len(lines) == 3 and re.match(r'^\d+$', lines[2].strip()):
            return True
    return False

def process_image(job_info):
    """Process a single image with mpeg7_app for all descriptors and parameter combinations"""
    global ERROR_COUNTER, ERROR_COUNTER_LOCK, ERROR_DETAILS, ERROR_DETAILS_LOCK, DESCRIPTOR_ERROR_COUNTS, DESCRIPTOR_ERROR_LOCK
    
    image_file, descriptor_id, params_dict, output_dir, mpeg7_app = job_info
    
    filename = os.path.basename(image_file)
    base_filename = os.path.splitext(filename)[0]
    descriptor_name = DESCRIPTORS[descriptor_id]["name"]
    
    # Create a unique identifier for this parameter combination
    params_str = "_".join([f"{k}-{v}" for k, v in params_dict.items()]) if params_dict else "default"
    output_file = os.path.join(output_dir, f"{base_filename}_{descriptor_name}_{params_str}.txt")
    
    # Build command parameters
    cmd = [mpeg7_app, str(descriptor_id), image_file]
    
    # Add parameters to command if any
    for param_name, param_value in params_dict.items():
        cmd.extend([param_name, param_value])
        
    try:
        # Run with full error output for debugging
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False
        )

        if result.returncode != 0:
            print(f"Failed to process {filename} with descriptor {descriptor_name}, params {params_dict}: {result.stderr.decode()}")
            return False

        output_text = result.stdout.decode()
        
        # Check if output is just a number instead of XML
        if is_numeric_output(output_text):
            with ERROR_COUNTER_LOCK:
                ERROR_COUNTER += 1
                # Update descriptor-specific counts for real-time tracking
                with DESCRIPTOR_ERROR_LOCK:
                    descriptor_key = f"{descriptor_id} ({descriptor_name})"
                    DESCRIPTOR_ERROR_COUNTS[descriptor_key] = DESCRIPTOR_ERROR_COUNTS.get(descriptor_key, 0) + 1
                    
                    # Print real-time summary every 10 errors or when this is the first error for a descriptor
                    if ERROR_COUNTER % 10 == 1 or DESCRIPTOR_ERROR_COUNTS[descriptor_key] == 1:
                        print("\n--- NUMERIC OUTPUT ERROR SUMMARY ---")
                        print(f"Total errors so far: {ERROR_COUNTER}")
                        for desc, count in sorted(DESCRIPTOR_ERROR_COUNTS.items(), key=lambda x: x[1], reverse=True):
                            print(f"  Descriptor {desc}: {count} errors")
                        print("-----------------------------------\n")
                
                error_message = f"ERROR #{ERROR_COUNTER}: {filename} with descriptor_id: {descriptor_id} ({descriptor_name}), params: {params_dict}"
                print(error_message)
                
                with ERROR_DETAILS_LOCK:
                    ERROR_DETAILS.append({
                        "image": filename,
                        "descriptor_id": descriptor_id,
                        "descriptor_name": descriptor_name,
                        "parameters": params_dict,
                        "output": output_text.strip()
                    })
            
            with DESCRIPTOR_ERROR_LOCK:
                DESCRIPTOR_ERROR_COUNTS[descriptor_id] = DESCRIPTOR_ERROR_COUNTS.get(descriptor_id, 0) + 1

        with open(output_file, 'w') as f:
            # Include metadata about the descriptor and parameters
            metadata = {
                "image": filename,
                "descriptor_id": descriptor_id,
                "descriptor_name": descriptor_name,
                "parameters": params_dict
            }
            f.write(json.dumps(metadata) + "\n")
            f.write(output_text)
        return True
    except Exception as e:
        print(f"Error processing {filename} with descriptor {descriptor_name}, params {params_dict}: {str(e)}")
        return False

def get_parameter_combinations(descriptor_params):
    """Generate all possible parameter combinations for a descriptor"""
    if not descriptor_params:
        return [{}]
    
    keys = descriptor_params.keys()
    values = descriptor_params.values()
    
    # Generate all combinations of parameter values
    combinations = []
    for combo in itertools.product(*values):
        combinations.append(dict(zip(keys, combo)))
    
    return combinations

def list_descriptors():
    """Print information about available descriptors and their parameters"""
    print("Available MPEG-7 Descriptors:")
    print("============================")
    for desc_id, desc_info in sorted(DESCRIPTORS.items()):
        desc_name = desc_info["name"]
        print(f"{desc_id}: {desc_name}")
        
        # Print parameter information
        for param_set in desc_info["params"]:
            if not param_set:
                print("  No configurable parameters")
            else:
                print("  Parameters:")
                for param_name, values in param_set.items():
                    print(f"    {param_name}: {', '.join(values)}")
        print()

def analyze_existing_results(output_dir):
    """Analyze existing output files without reprocessing images"""
    global ERROR_COUNTER, ERROR_DETAILS, DESCRIPTOR_ERROR_COUNTS
    
    print(f"Analyzing existing output files in {output_dir}...")
    output_files = glob.glob(f"{output_dir}/*.txt")
    
    for txt_file in tqdm(output_files, desc="Checking files", unit="file"):
        try:
            with open(txt_file, "r") as infile:
                content = infile.read().strip()
                
                if is_numeric_output(content):
                    ERROR_COUNTER += 1
                    
                    # Try to extract metadata
                    lines = content.split('\n')
                    if lines and lines[0].startswith('{'):
                        try:
                            metadata = json.loads(lines[0])
                            image = metadata.get('image', os.path.basename(txt_file))
                            desc_id = metadata.get('descriptor_id', 'unknown')
                            desc_name = metadata.get('descriptor_name', 'unknown')
                            params = metadata.get('parameters', {})
                            
                            # Update descriptor-specific counts
                            descriptor_key = f"{desc_id} ({desc_name})"
                            DESCRIPTOR_ERROR_COUNTS[descriptor_key] = DESCRIPTOR_ERROR_COUNTS.get(descriptor_key, 0) + 1
                            
                            # Print real-time summary every 10 errors or when this is the first error for a descriptor
                            if ERROR_COUNTER % 10 == 1 or DESCRIPTOR_ERROR_COUNTS[descriptor_key] == 1:
                                print("\n--- NUMERIC OUTPUT ERROR SUMMARY ---")
                                print(f"Total errors so far: {ERROR_COUNTER}")
                                for d, c in sorted(DESCRIPTOR_ERROR_COUNTS.items(), key=lambda x: x[1], reverse=True):
                                    print(f"  Descriptor {d}: {c} errors")
                                print("-----------------------------------\n")
                            
                            error_message = f"ERROR #{ERROR_COUNTER}: {image} with descriptor_id: {desc_id} ({desc_name}), params: {params}"
                            print(error_message)
                            
                            ERROR_DETAILS.append({
                                "file": os.path.basename(txt_file),
                                "image": image,
                                "descriptor_id": desc_id,
                                "descriptor_name": desc_name,
                                "parameters": params,
                                "output": content
                            })
                        except json.JSONDecodeError:
                            print(f"ERROR #{ERROR_COUNTER}: {os.path.basename(txt_file)} (metadata parsing failed)")
        except Exception as e:
            print(f"Error reading {txt_file}: {str(e)}")
    
    return ERROR_COUNTER

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description="Process images with MPEG-7 descriptors")
    parser.add_argument("--descriptors", type=int, nargs="+", 
                      help="Specific descriptor IDs to process (1-10). If not specified, all descriptors are processed.")
    parser.add_argument("--dataset", type=str, default=DATASET_DIR,
                      help=f"Path to dataset directory. Default: {DATASET_DIR}")
    parser.add_argument("--output", type=str, default=OUTPUT_DIR,
                      help=f"Path to output directory. Default: {OUTPUT_DIR}")
    parser.add_argument("--jobs", type=int, default=MAX_JOBS,
                      help=f"Number of parallel jobs. Default: {MAX_JOBS}")
    parser.add_argument("--mpeg7app", type=str, default=MPEG7_APP,
                      help=f"Path to MPEG-7 application. Default: {MPEG7_APP}")
    parser.add_argument("--skip-combine", action="store_true",
                      help="Skip combining results into a single file")
    parser.add_argument("--list-descriptors", action="store_true",
                      help="List available descriptors and their parameters, then exit")
    parser.add_argument("--analyze-only", action="store_true",
                      help="Only analyze existing output files without processing images")
    args = parser.parse_args()
    
    # List descriptors if requested
    if args.list_descriptors:
        list_descriptors()
        sys.exit(0)
    
    # Update paths based on command-line arguments
    output_dir = args.output
    
    # If we're only analyzing existing results, do that and exit
    if args.analyze_only:
        if not os.path.isdir(output_dir):
            print(f"Error: Output directory '{output_dir}' does not exist.")
            sys.exit(1)
        
        total_errors = analyze_existing_results(output_dir)
        
        # Print summary of numeric outputs
        print("\n" + "="*60)
        print(f" FINAL SUMMARY: {total_errors} outputs with numeric values instead of XML")
        print("="*60)
        
        if total_errors > 0:
            # Save error details to a file for later analysis
            error_log_file = "numeric_output_errors.json"
            with open(error_log_file, 'w') as f:
                json.dump(ERROR_DETAILS, f, indent=2)
            print(f"Error details saved to {error_log_file}")
            
            # Print statistics about which descriptors had the most errors
            print("\nError distribution by descriptor:")
            for desc, count in sorted(DESCRIPTOR_ERROR_COUNTS.items(), key=lambda x: x[1], reverse=True):
                print(f"  Descriptor {desc}: {count} errors")
                
            # Print detailed parameter analysis for the top error-prone descriptors
            print("\nDetailed parameter analysis for top descriptors:")
            descriptor_param_counts = {}
            for error in ERROR_DETAILS:
                desc_id = error["descriptor_id"]
                desc_name = error["descriptor_name"]
                params = error["parameters"]
                
                key = f"{desc_id} ({desc_name})"
                if key not in descriptor_param_counts:
                    descriptor_param_counts[key] = {}
                    
                # Convert parameters to string for counting
                param_str = json.dumps(params, sort_keys=True)
                descriptor_param_counts[key][param_str] = descriptor_param_counts[key].get(param_str, 0) + 1
            
            # Show parameter details for top 3 descriptors with errors
            for desc, _ in sorted(DESCRIPTOR_ERROR_COUNTS.items(), key=lambda x: x[1], reverse=True)[:3]:
                if desc in descriptor_param_counts:
                    print(f"\n  {desc} parameter breakdown:")
                    for param_str, count in sorted(descriptor_param_counts[desc].items(), key=lambda x: x[1], reverse=True):
                        params = json.loads(param_str)
                        print(f"    Parameters {params}: {count} errors")
        
        sys.exit(0)
    
    # For normal processing flow
    dataset_dir = args.dataset
    max_jobs = args.jobs
    mpeg7_app = args.mpeg7app
    
    # Filter descriptors if specified, leave only TEXTURE_BROWSING
    descriptors_to_process = DESCRIPTORS
    if args.descriptors:
        descriptors_to_process = {d_id: DESCRIPTORS[d_id] for d_id in args.descriptors if d_id in DESCRIPTORS}
        if not descriptors_to_process:
            print("Error: No valid descriptor IDs provided.")
            parser.print_help()
            sys.exit(1)
        print(f"Processing with selected descriptors: {list(descriptors_to_process.keys())}")
    
    # Check if dataset directory exists
    if not os.path.isdir(dataset_dir):
        print(f"Error: Directory '{dataset_dir}' does not exist.")
        sys.exit(1)

    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Get all files in the dataset directory
    image_files = []
    for root, _, files in os.walk(dataset_dir):
        for file in files:
            if file.lower().endswith(('.jpg', '.jpeg', '.png', '.bmp', '.pgm', '.ppm')):  # Filter image files
                image_files.append(os.path.join(root, file))

    print(f"Found {len(image_files)} image files to process.")
    
    # Exit if no images found
    if not image_files:
        print("No image files found in the dataset directory. Exiting.")
        sys.exit(1)
        
    # Create a list of jobs (image, descriptor, parameter combinations, output_dir, mpeg7_app)
    jobs = []
    for image_file in image_files:
        for descriptor_id, descriptor_info in descriptors_to_process.items():
            for param_set in descriptor_info["params"]:
                param_combinations = get_parameter_combinations(param_set)
                for params in param_combinations:
                    jobs.append((image_file, descriptor_id, params, output_dir, mpeg7_app))
    
    total_jobs = len(jobs)
    print(f"Total job combinations: {total_jobs} (images × descriptors × parameter combinations)")
    print(f"Starting parallel processing using {max_jobs} jobs...")
    
    # Start timing
    start_time = time.time()

    # Process files with progress indicator
    with ProcessPoolExecutor(max_workers=max_jobs) as executor:
        # Use tqdm for progress bar that updates in-place
        results = list(tqdm(
            executor.map(process_image, jobs),
            total=total_jobs,
            unit="job"
        ))

    # Calculate processing time
    processing_time = time.time() - start_time
    
    successful = results.count(True)
    failed = results.count(False)

    if not args.skip_combine:
        print(f"\nCombining results into a single file...")

        # Combine results into descriptor-specific files and a master file
        all_results_file = "combined_results.txt"
        descriptor_results = {}
        
        # First initialize descriptor result files
        for descriptor_id, descriptor_info in descriptors_to_process.items():
            descriptor_name = descriptor_info["name"]
            descriptor_results[descriptor_id] = f"combined_{descriptor_name.lower()}_results.txt"
            
            # Create empty descriptor files
            with open(descriptor_results[descriptor_id], 'w') as f:
                f.write(f"# Combined results for {descriptor_name} descriptor\n\n")
        
        # Now write the combined results
        with open(all_results_file, "w") as all_outfile:
            all_outfile.write("# Combined results for all descriptors\n\n")
            
            for txt_file in sorted(glob.glob(f"{output_dir}/*.txt")):
                try:
                    with open(txt_file, "r") as infile:
                        content = infile.read()
                        
                        # Try to extract the metadata from the first line
                        lines = content.strip().split('\n')
                        if lines and lines[0].startswith('{'):
                            try:
                                metadata = json.loads(lines[0])
                                descriptor_id = metadata.get('descriptor_id')
                                
                                # Write to the descriptor-specific file
                                if descriptor_id in descriptor_results:
                                    with open(descriptor_results[descriptor_id], 'a') as desc_file:
                                        desc_file.write(f"# File: {os.path.basename(txt_file)}\n")
                                        desc_file.write(content)
                                        desc_file.write("\n\n")
                            except json.JSONDecodeError:
                                pass
                        
                        # Write to the all-results file
                        all_outfile.write(f"# File: {os.path.basename(txt_file)}\n")
                        all_outfile.write(content)
                        all_outfile.write("\n\n")
                except Exception as e:
                    print(f"Error reading {txt_file}: {str(e)}")
    
    print(f"\nProcessing complete in {timedelta(seconds=int(processing_time))}.")
    print(f"{successful} jobs completed successfully, {failed} jobs failed.")
    
    # Print summary of numeric outputs
    print("\n" + "="*60)
    print(f" FINAL SUMMARY: {ERROR_COUNTER} outputs with numeric values instead of XML")
    print("="*60)
    
    if ERROR_COUNTER > 0:
        # Save error details to a file for later analysis
        error_log_file = "numeric_output_errors.json"
        with open(error_log_file, 'w') as f:
            json.dump(ERROR_DETAILS, f, indent=2)
        print(f"Error details saved to {error_log_file}")
        
        # Print statistics about which descriptors had the most errors
        print("\nError distribution by descriptor:")
        for desc, count in sorted(DESCRIPTOR_ERROR_COUNTS.items(), key=lambda x: x[1], reverse=True):
            print(f"  Descriptor {desc}: {count} errors")
            
        # Print detailed parameter analysis for the top error-prone descriptors
        print("\nDetailed parameter analysis for top descriptors:")
        descriptor_param_counts = {}
        for error in ERROR_DETAILS:
            desc_id = error["descriptor_id"]
            desc_name = error["descriptor_name"]
            params = error["parameters"]
            
            key = f"{desc_id} ({desc_name})"
            if key not in descriptor_param_counts:
                descriptor_param_counts[key] = {}
                
            # Convert parameters to string for counting
            param_str = json.dumps(params, sort_keys=True)
            descriptor_param_counts[key][param_str] = descriptor_param_counts[key].get(param_str, 0) + 1
        
        # Show parameter details for top 3 descriptors with errors
        for desc, _ in sorted(DESCRIPTOR_ERROR_COUNTS.items(), key=lambda x: x[1], reverse=True)[:3]:
            print(f"\n  {desc} parameter breakdown:")
            for param_str, count in sorted(descriptor_param_counts[desc].items(), key=lambda x: x[1], reverse=True):
                params = json.loads(param_str)
                print(f"    Parameters {params}: {count} errors")
    
    print(f"\nIndividual results saved to {output_dir}/")
    
    if not args.skip_combine:
        print(f"Combined results saved to {all_results_file}")
        print(f"Descriptor-specific results saved to combined_*_results.txt files")


if __name__ == "__main__":
    main()