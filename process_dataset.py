#!/usr/bin/env python3

import os
import sys
import subprocess
import tqdm
import glob
import itertools
import json
import argparse
import time
import math
from datetime import timedelta

ONLY_DESCRIPTOR_ID = 10

DATASET_DIR = "/home/kuchkr/Desktop/dataset/png"
OUTPUT_DIR = "/home/kuchkr/Desktop/original_mpeg7_files/out"
MPEG7_APP = "./build/x64/Release/mpeg7_app"

ALL_DESCRIPTORS = {
    1: {
        "name": "DOMINANT_COLOR",
        "params": [
            {"VariancePresent": ["1"], "SpatialCoherency": ["1"]}
        ]
    },
    2: {
        "name": "SCALABLE_COLOR",
        "params": [
            {"NumberOfCoefficients": ["256"], 
             "NumberOfBitplanesDiscarded": ["4"] }
        ]
    },
    3: {
        "name": "COLOR_LAYOUT",
        "params": [
            {"NumberOfYCoeff": ["64"],
                "NumberOfCCoeff": ["64"],
            }
        ]
    },
    4: {
        "name": "COLOR_STRUCTURE",
        "params": [
            {"ColorQuantSize": ["256"] }
        ]
    },
    5: {
        "name": "CT_BROWSING",
        "params": [
            {}
        ]
    },
    6: {
        "name": "HOMOGENEOUS_TEXTURE",
        "params": [
            {"layer": ["1"]}
        ]
    },
    7: {
        "name": "TEXTURE_BROWSING",
        "params": [
            {
                "layer": ["1"],
            }
        ]
    },
    8: {
        "name": "EDGE_HISTOGRAM",
        "params": [
            {}
        ]
    },
    9: {
        "name": "REGION_SHAPE",
        "params": [
            {}
        ]
    },
    10: {
        "name": "CONTOUR_SHAPE",
        "params": [
            {}
        ]
    }
}

def get_available_descriptors():
    # If ONLY_DESCRIPTOR_ID is set, return only that descriptor
    if ONLY_DESCRIPTOR_ID and ONLY_DESCRIPTOR_ID in ALL_DESCRIPTORS:
        return {str(ONLY_DESCRIPTOR_ID): ALL_DESCRIPTORS[ONLY_DESCRIPTOR_ID]}
    # Otherwise return all descriptors
    return {str(desc_id): desc_info for desc_id, desc_info in ALL_DESCRIPTORS.items()}

def process_image(image_file, descriptor_id, params_dict, output_dir, mpeg7_app):
    filename = os.path.basename(image_file)
    base_filename = os.path.splitext(filename)[0]
    descriptor_name = ALL_DESCRIPTORS[int(descriptor_id)]["name"]
    
    params_str = "_".join([f"{k}-{v}" for k, v in params_dict.items()]) if params_dict else "default"
    output_file = os.path.join(output_dir, f"{base_filename}_{descriptor_name}_{params_str}.txt")
    
    cmd = [mpeg7_app, str(descriptor_id), image_file]
    
    for param_name, param_value in params_dict.items():
        cmd.extend([param_name, param_value])
        
    try:
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False
        )

        if result.returncode != 0:
            print(f"Failed to process {filename} with descriptor {descriptor_name}, params {params_dict}: {result.stderr.decode()}")
            return False

        output_text = result.stdout.decode()
        
        with open(output_file, 'w') as f:
            metadata = {
                "image": filename,
                "descriptor_id": int(descriptor_id),
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
    if not descriptor_params:
        return [{}]
    
    keys = descriptor_params.keys()
    values = descriptor_params.values()
    
    combinations = []
    for combo in itertools.product(*values):
        combinations.append(dict(zip(keys, combo)))
    
    return combinations

def main():
    global ONLY_DESCRIPTOR_ID
    
    output_dir = OUTPUT_DIR
    dataset_dir = DATASET_DIR
    mpeg7_app = MPEG7_APP
    
    available_descriptors = get_available_descriptors()
    descriptors_to_process = {}
    for desc_id, desc_info in available_descriptors.items():
        descriptors_to_process[desc_id] = {
            "name": desc_info["name"],
            "params": desc_info["params"]
        }
    
    if not descriptors_to_process:
        print("Error: No valid descriptors available.")
        sys.exit(1)
        
    desc_ids = list(descriptors_to_process.keys())
    if ONLY_DESCRIPTOR_ID:
        print(f"Processing with single descriptor: {desc_ids[0]}")
    else:
        print(f"Processing with all available descriptors: {desc_ids}")
    
    if not os.path.isdir(dataset_dir):
        print(f"Error: Directory '{dataset_dir}' does not exist.")
        sys.exit(1)

    os.makedirs(output_dir, exist_ok=True)
    
    image_files = []
    for root, _, files in os.walk(dataset_dir):
        for file in files:
            if file.lower().endswith(('.jpg', '.jpeg', '.png', '.bmp', '.pgm', '.ppm')):
                image_files.append(os.path.join(root, file))

    print(f"Found {len(image_files)} image files to process.")
    
    if not image_files:
        print("No image files found in the dataset directory. Exiting.")
        sys.exit(1)
        
    jobs = []
    for image_file in image_files:
        for descriptor_id, descriptor_info in descriptors_to_process.items():
            for param_set in descriptor_info["params"]:
                param_combinations = get_parameter_combinations(param_set)
                for params in param_combinations:
                    jobs.append((image_file, descriptor_id, params, output_dir, mpeg7_app))
    
    total_jobs = len(jobs)
    print(f"Total job combinations: {total_jobs} (images × descriptors × parameter combinations)")
    
    start_time = time.time()
    successful = 0
    failed = 0
    
    print("Starting sequential processing...")
    
    try:
        results = []
        for job in tqdm.tqdm(jobs, desc="Processing images"):
            image_file, descriptor_id, params, output_dir, mpeg7_app = job
            result = process_image(image_file, descriptor_id, params, output_dir, mpeg7_app)
            results.append(result)
            if result:
                successful += 1
            else:
                failed += 1
    except KeyboardInterrupt:
        print("\nProcessing interrupted by user.")
        sys.exit(1)
    
    processing_time = time.time() - start_time
    
    print(f"\nProcessing complete in {timedelta(seconds=int(processing_time))}.")
    print(f"{successful} jobs completed successfully, {failed} jobs failed.")
    print(f"\nIndividual results saved to {output_dir}/")


if __name__ == "__main__":
    main()
