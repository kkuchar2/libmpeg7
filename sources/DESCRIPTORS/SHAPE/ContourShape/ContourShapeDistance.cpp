#include "ContourShapeDistance.h"

ContourShapeDistance::ContourShapeDistance() {
    m_rPeaksX = new float[(CONTOURSHAPE_CSSPEAKMASK) + 1];
    m_rPeaksY = new float[(CONTOURSHAPE_CSSPEAKMASK) + 1];
    m_qPeaksX = new float[(CONTOURSHAPE_CSSPEAKMASK) + 1];
    m_qPeaksY = new float[(CONTOURSHAPE_CSSPEAKMASK) + 1];

    m_nodeList = new Node[4 * ((CONTOURSHAPE_CSSPEAKMASK) + 1) * ((CONTOURSHAPE_CSSPEAKMASK) + 1)];
}

double ContourShapeDistance::getDistance(Descriptor * descriptor1, Descriptor * descriptor2, const char ** params) {
    const auto contourShapeDescriptor1 = static_cast<ContourShape *>(descriptor1);
    const auto contourShapeDescriptor2 = static_cast<ContourShape *>(descriptor2);

    const double distance = getDistance(contourShapeDescriptor1, contourShapeDescriptor2, params);

    if (distance == DBL_MAX) {
        throw CONT_SHAPE_DISTANCE_ERROR;
    }

    return distance;
}

double ContourShapeDistance::getDistance(ContourShape *  contourShapeDescriptor1, ContourShape * contourShapeDescriptor2, const char ** params) {
    unsigned long lRefE, lRefC, lQueryE, lQueryC;

    contourShapeDescriptor1->GetGlobalCurvature(lRefC, lRefE);
    contourShapeDescriptor2->GetGlobalCurvature(lQueryC, lQueryE);

    float fRefE = static_cast<float>(0.5 + lRefE);
    float fRefC = static_cast<float>(0.5 + lRefC);
    float fQueryE = static_cast<float>(0.5 + lQueryE);
    float fQueryC = static_cast<float>(0.5 + lQueryC);

    float fDenomE = (fRefE > fQueryE) ? fRefE : fQueryE;
    float fDenomC = (fRefC > fQueryC) ? fRefC : fQueryC;

    fDenomE += static_cast<float>(CONTOURSHAPE_EMIN * (CONTOURSHAPE_EMASK + 1) / (CONTOURSHAPE_EMAX - CONTOURSHAPE_EMIN));
    fDenomC += static_cast<float>(CONTOURSHAPE_CMIN * (CONTOURSHAPE_CMASK + 1) / (CONTOURSHAPE_CMAX - CONTOURSHAPE_CMIN));

    if ((fabs(fRefE - fQueryE) >= CONTOURSHAPE_ETHR * fDenomE) || (fabs(fRefC - fQueryC) >= CONTOURSHAPE_CTHR * fDenomC)) {
        return DBL_MAX;
    }

    float tCost = static_cast<float>((CONTOURSHAPE_ECOST * fabs(fRefE - fQueryE) / fDenomE) + (CONTOURSHAPE_CCOST * fabs(fRefC - fQueryC) / fDenomC));

    int nRefPeaks = contourShapeDescriptor1->GetNumberOfPeaks();
    int nQueryPeaks = contourShapeDescriptor2->GetNumberOfPeaks();

    for (int nr = 0; nr < nRefPeaks; nr++) {
        unsigned short irx, iry;

        contourShapeDescriptor1->GetPeak(nr, irx, iry);

        m_rPeaksX[nr] = static_cast<float>((irx * CONTOURSHAPE_XMAX / static_cast<float> (CONTOURSHAPE_XMASK)));

        if (nr == 0) {
            m_rPeaksY[nr] = static_cast<float>((iry * CONTOURSHAPE_YMAX / static_cast<float> (CONTOURSHAPE_YMASK)));
        }
        else {
            m_rPeaksY[nr] = iry * m_rPeaksY[nr - 1] / static_cast<float> (CONTOURSHAPE_YnMASK);
        }
    }

    for (int nq = 0; nq < nQueryPeaks; nq++) {
        unsigned short iqx, iqy;

        contourShapeDescriptor2->GetPeak(nq, iqx, iqy);

        m_qPeaksX[nq] = static_cast<float>((iqx * CONTOURSHAPE_XMAX / static_cast<float> (CONTOURSHAPE_XMASK)));

        if (nq == 0) {
            m_qPeaksY[nq] = static_cast<float>((iqy * CONTOURSHAPE_YMAX / static_cast<float> (CONTOURSHAPE_YMASK)));
        }
        else {
            m_qPeaksY[nq] = (iqy * m_qPeaksY[nq - 1] / static_cast<float> (CONTOURSHAPE_YnMASK));
        }
    }

    int nNodes = 0;

    for (int i0 = 0; (i0 < nRefPeaks) && (i0 < CONTOURSHAPE_NMATCHPEAKS); i0++) {

        float iRefX = m_rPeaksX[i0];
        float iRefY = m_rPeaksY[i0];

        for (int j0 = 0; j0 < nQueryPeaks; j0++) {

            float iQueryX = m_qPeaksX[j0];
            float iQueryY = m_qPeaksY[j0];

            float denom = (iRefY > iQueryY) ? iRefY : iQueryY;

            if ((fabs(iRefY - iQueryY) / denom) < 0.7) {

                m_nodeList[nNodes].cost = 0.0;
                m_nodeList[nNodes].nr = nRefPeaks;
                m_nodeList[nNodes].nq = nQueryPeaks;

                m_nodeList[nNodes + 1].cost = 0.0;
                m_nodeList[nNodes + 1].nr = nRefPeaks;
                m_nodeList[nNodes + 1].nq = nQueryPeaks;

                for (int pr = 0; pr < nRefPeaks; pr++) {

                    float frx = range(m_rPeaksX[pr] - iRefX);
                    float fry = m_rPeaksY[pr];

                    m_nodeList[nNodes].rPeaks[pr].x = frx;
                    m_nodeList[nNodes].rPeaks[pr].y = fry;

                    m_nodeList[nNodes + 1].rPeaks[pr].x = frx;
                    m_nodeList[nNodes + 1].rPeaks[pr].y = fry;
                }

                for (int pq = 0; pq < nQueryPeaks; pq++) {

                    float fqx = range(m_qPeaksX[pq] - iQueryX);
                    float fqy = m_qPeaksY[pq];

                    m_nodeList[nNodes].qPeaks[pq].x = fqx;
                    m_nodeList[nNodes].qPeaks[pq].y = fqy;

                    m_nodeList[nNodes + 1].qPeaks[pq].x = (range(-1.0f * fqx));
                    m_nodeList[nNodes + 1].qPeaks[pq].y = fqy;
                }
                nNodes += 2;
            }
        }
    }

    if (nRefPeaks == 0) {

        m_nodeList[nNodes].cost = 0.0;
        m_nodeList[nNodes].nr = nRefPeaks;
        m_nodeList[nNodes].nq = nQueryPeaks;

        for (int pq = 0; pq < nQueryPeaks; pq++) {

            float fqx = m_qPeaksX[pq];
            float fqy = m_qPeaksY[pq];

            m_nodeList[nNodes].qPeaks[pq].x = fqx;
            m_nodeList[nNodes].qPeaks[pq].y = fqy;
        }

        nNodes++;
    }

    for (int i1 = 0; (i1 < nQueryPeaks) && (i1 < CONTOURSHAPE_NMATCHPEAKS); i1++) {
        float iQueryX = m_qPeaksX[i1];
        float iQueryY = m_qPeaksY[i1];

        for (int j1 = 0; j1 < nRefPeaks; j1++) {
            float iRefX = m_rPeaksX[j1];
            float iRefY = m_rPeaksY[j1];

            float denom = (iQueryY > iRefY) ? iQueryY : iRefY;

            if ((fabs(iQueryY - iRefY) / denom) < 0.7) {

                m_nodeList[nNodes].cost = 0.0;
                m_nodeList[nNodes].nr = nQueryPeaks;
                m_nodeList[nNodes].nq = nRefPeaks;

                m_nodeList[nNodes + 1].cost = 0.0;
                m_nodeList[nNodes + 1].nr = nQueryPeaks;
                m_nodeList[nNodes + 1].nq = nRefPeaks;

                for (int pq = 0; pq < nQueryPeaks; pq++) {
                    float fqx = range(m_qPeaksX[pq] - iQueryX);
                    float fqy = m_qPeaksY[pq];

                    m_nodeList[nNodes].rPeaks[pq].x = fqx;
                    m_nodeList[nNodes].rPeaks[pq].y = fqy;

                    m_nodeList[nNodes + 1].rPeaks[pq].x = range(-1.0f * fqx);
                    m_nodeList[nNodes + 1].rPeaks[pq].y = fqy;
                }

                for (int pr = 0; pr < nRefPeaks; pr++) {
                    float frx = range(m_rPeaksX[pr] - iRefX);
                    float fry = m_rPeaksY[pr];

                    m_nodeList[nNodes].qPeaks[pr].x = frx;
                    m_nodeList[nNodes].qPeaks[pr].y = fry;

                    m_nodeList[nNodes + 1].qPeaks[pr].x = frx;
                    m_nodeList[nNodes + 1].qPeaks[pr].y = fry;
                }
                nNodes += 2;
            }
        }
    }

    if (nQueryPeaks == 0) {
        m_nodeList[nNodes].cost = 0.0;
        m_nodeList[nNodes].nr = nQueryPeaks;
        m_nodeList[nNodes].nq = nRefPeaks;

        for (int pr = 0; pr < nRefPeaks; pr++) {
            float frx = m_rPeaksX[pr];
            float fry = m_rPeaksY[pr];

            m_nodeList[nNodes].qPeaks[pr].x = frx;
            m_nodeList[nNodes].qPeaks[pr].y = fry;
        }

        nNodes++;
    }

    if (nNodes == 0) {
        return DBL_MAX;
    }

    int index = 0;

    while ((m_nodeList[index].nr > 0) || (m_nodeList[index].nq > 0)) {
        int ir = -1, iq = -1;

        if ((m_nodeList[index].nr > 0) && (m_nodeList[index].nq > 0)) {

            ir = 0;
            for (int mr = 1; mr < m_nodeList[index].nr; mr++) {

                if (m_nodeList[index].rPeaks[ir].y < m_nodeList[index].rPeaks[mr].y)
                    ir = mr;
            }

            iq = 0;

            float xd = static_cast<float>(fabs(m_nodeList[index].rPeaks[ir].x - m_nodeList[index].qPeaks[iq].x));

            if (xd > 0.5) {
                xd = 1.0f - xd;
            }

            float yd = static_cast<float>(fabs(m_nodeList[index].rPeaks[ir].y - m_nodeList[index].qPeaks[iq].y));
            float sqd = xd * xd + yd * yd;

            for (int mq = 1; mq < m_nodeList[index].nq; mq++) {
                xd = static_cast<float>(fabs(m_nodeList[index].rPeaks[ir].x - m_nodeList[index].qPeaks[mq].x));

                if (xd > 0.5) {
                    xd = 1.0f - xd;
                }

                yd = static_cast<float>(fabs(m_nodeList[index].rPeaks[ir].y - m_nodeList[index].qPeaks[mq].y));

                float d = xd * xd + yd * yd;

                if (d < sqd) {
                    sqd = d;
                    iq = mq;
                }
            }

            float dx = static_cast<float>(fabs(m_nodeList[index].rPeaks[ir].x - m_nodeList[index].qPeaks[iq].x));

            if (dx > 0.5) {
                dx = 1.0f - dx;
            }

            if (dx < 0.1) {
                float dy = static_cast<float>(fabs(m_nodeList[index].rPeaks[ir].y - m_nodeList[index].qPeaks[iq].y));

                m_nodeList[index].cost += sqrt(dx * dx + dy * dy);

                if (ir < --m_nodeList[index].nr) {
                    memmove(&m_nodeList[index].rPeaks[ir], &m_nodeList[index].rPeaks[ir + 1], (m_nodeList[index].nr - ir) * sizeof(m_nodeList[index].rPeaks[0]));
                }

                if (iq < --m_nodeList[index].nq) {
                    memmove(&m_nodeList[index].qPeaks[iq], &m_nodeList[index].qPeaks[iq + 1], (m_nodeList[index].nq - iq) * sizeof(m_nodeList[index].qPeaks[0]));
                }
            }
            else {
                m_nodeList[index].cost += m_nodeList[index].rPeaks[ir].y;

                if (ir < --m_nodeList[index].nr) {
                    memmove(&m_nodeList[index].rPeaks[ir], &m_nodeList[index].rPeaks[ir + 1], (m_nodeList[index].nr - ir)*sizeof(m_nodeList[index].rPeaks[0]));
                }
            }
        }
        else if (m_nodeList[index].nr > 0) {

            m_nodeList[index].cost += m_nodeList[index].rPeaks[0].y;

            if (--m_nodeList[index].nr > 0) {
                memmove(&m_nodeList[index].rPeaks[0], &m_nodeList[index].rPeaks[1], (m_nodeList[index].nr) * sizeof(m_nodeList[index].rPeaks[0]));
            }
        }
        else { // if (m_nodeList[index].nq > 0)

            m_nodeList[index].cost += m_nodeList[index].qPeaks[0].y;

            if (--m_nodeList[index].nq > 0) {
                memmove(&m_nodeList[index].qPeaks[0], &m_nodeList[index].qPeaks[1], (m_nodeList[index].nq)*sizeof(m_nodeList[index].qPeaks[0]));
            }
        }

        index = 0;

        double minCost = m_nodeList[index].cost;

        for (int c0 = 1; c0 < nNodes; c0++) {

            if (m_nodeList[c0].cost < minCost) {
                index = c0;
                minCost = m_nodeList[c0].cost;
            }
        }
    }

    double cost = m_nodeList[index].cost + tCost;

    return cost;
}

float ContourShapeDistance::range(float x) {
    while (x < 0.0)  x += 1.0;
    while (x >= 1.0) x -= 1.0;
    return x;
}

ContourShapeDistance::~ContourShapeDistance() {
    if (m_rPeaksX) {
        delete[] m_rPeaksX;
    }

    if (m_rPeaksY) {
        delete[] m_rPeaksY;
    }

    if (m_qPeaksX) {
        delete[] m_qPeaksX;
    }

    if (m_qPeaksY) {
        delete[] m_qPeaksY;
    }

    if (m_nodeList) {
        delete[] m_nodeList;
    }
}