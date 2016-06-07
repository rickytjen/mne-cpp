//=============================================================================================================
/**
* @file     spline.cpp
* @author   Ricky Tjen <ricky270@student.sgu.ac.id>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>;
* @version  1.0
* @date     April, 2016
*
* @section  LICENSE
*
* Copyright (C) 2016, Ricky Tjen and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    implementation of spline class
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include <spline.h>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsTextItem>
#include <QtGui/QMouseEvent>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace DISPLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

Spline::Spline(const QString& title, QWidget* parent): QWidget(parent)
{
    m_pChart = new QChart();
    m_pChart->setTitle(title);
    m_pChart->setAnimationOptions(QChart::SeriesAnimations);
    m_pChart->setAcceptHoverEvents(false);
    series = new QSplineSeries();
    QChartView *chartView = new QChartView(m_pChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(chartView, 0, 0);
    this->setLayout(layout);
}


//*************************************************************************************************************

void Spline::mousePressEvent(QMouseEvent *event)
{
    if (series->count() == 0)               //protect integrity of the histogram widget in case series contain no data values
    {
        qDebug() << "Data set not found.";  //do nothing
    }

    else
    {
        QXYSeries *shadowSeries = qobject_cast<QXYSeries *>(sender());
        QLineSeries *verticalLine = new QLineSeries();
        QPointF point = event->pos();
        QPointF pointY = point;
        pointY.setX(minAxisX);
        pointY.setY(pointY.y()-10.5);   //-10.5 needed to correctly position the line at mouse position
        point.setX(point.x()-10.5);
        point.setY(0);

        QPointF localX = m_pChart->mapToValue(point, shadowSeries);
        QPointF localY = m_pChart->mapToValue(pointY, shadowSeries);
        verticalLine->append(localX.x(), 0);
        verticalLine->append(localX.x(), maximumFrequency);
        double boundaryX = double(localX.x());   //casting localX.x() from float to double for comparison with minAxisX and maxAxisX
        double boundaryY = double(localY.y());   //casting localY.y() from float to double for comparison with 0 and maximumFrequency

        if((boundaryX >= float(minAxisX)) && (boundaryX <= float(maxAxisX)))  //this condition ensures that threshold lines can only be created within the boundary of the x-axis
        {
            if((boundaryY >= 0.0) && (boundaryY <= float(maximumFrequency)))  // this condition ensures that threshold lines can only be created within the boundary of the y-axis
            {
                QVector<QPointF> middlePoint = middleThreshold->pointsVector();   //Point values need to be updated before tested and displayed on the widget
                QVector<QPointF> rightPoint = rightThreshold->pointsVector();
                QVector<QPointF> leftPoint = leftThreshold->pointsVector();

                double emitLeft = (leftPoint[0].x() * (pow(10, resultExponentValues[0])));
                double emitMiddle = (middlePoint[0].x() * (pow(10, resultExponentValues[0])));
                double emitRight = (rightPoint[0].x() * (pow(10, resultExponentValues[0])));

                if (event->buttons() == Qt::LeftButton)
                {
                    leftPoint = verticalLine->pointsVector();
                    if((leftPoint[0].x() < middlePoint[0].x()) && (leftPoint[0].x() < rightPoint[0].x()))
                    {
                        m_pChart->removeSeries(leftThreshold);
                        leftThreshold=verticalLine;
                        leftThreshold->setColor("red");
                        m_pChart->addSeries(leftThreshold);
                        m_pChart->legend()->markers().at(m_pChart->legend()->markers().size()-1)->setVisible(false);
                        m_pChart->createDefaultAxes();
                        emitLeft = (leftPoint[0].x() * (pow(10, resultExponentValues[0])));
                        emit borderChanged(emitLeft, emitMiddle, emitRight);
                        qDebug() << "Border = " << emitLeft << " , " << emitMiddle << " , " << emitRight;
                    }
                }

                if (event->buttons() == Qt::MiddleButton)
                {
                    middlePoint = verticalLine->pointsVector();
                    if((middlePoint[0].x() > leftPoint[0].x()) && (middlePoint[0].x() < rightPoint[0].x()))
                    {
                        m_pChart->removeSeries(middleThreshold);
                        middleThreshold=verticalLine;
                        middleThreshold->setColor("green");
                        m_pChart->addSeries(middleThreshold);
                        m_pChart->legend()->markers().at(m_pChart->legend()->markers().size()-1)->setVisible(false);
                        m_pChart->createDefaultAxes();
                        emitMiddle = (middlePoint[0].x() * (pow(10, resultExponentValues[0])));
                        emit borderChanged(emitLeft, emitMiddle, emitRight);
                        qDebug() << "Border = " << emitLeft << " , " << emitMiddle << " , " << emitRight;
                    }
                }

                if (event->buttons() == Qt::RightButton)
                {
                    rightPoint = verticalLine->pointsVector();
                    if((rightPoint[0].x() > leftPoint[0].x()) && (rightPoint[0].x() > middlePoint[0].x()))
                    {
                        m_pChart->removeSeries(rightThreshold);
                        rightThreshold=verticalLine;
                        rightThreshold->setColor("blue");
                        m_pChart->addSeries(rightThreshold);
                        m_pChart->legend()->markers().at(m_pChart->legend()->markers().size()-1)->setVisible(false);
                        m_pChart->createDefaultAxes();
                        emitRight = (rightPoint[0].x() * (pow(10, resultExponentValues[0])));
                        emit borderChanged(emitLeft, emitMiddle, emitRight);
                        qDebug() << "Border = " << emitLeft << " , " << emitMiddle << " , " << emitRight;
                    }
                }
            }
        }
    }
}


//*************************************************************************************************************

void Spline::setThreshold(const QVector3D& vecThresholdValues)
{
    float leftThresholdValue;
    float middleThresholdValue;
    float rightThresholdValue;

    if (series->count() == 0)               //protect integrity of the histogram widget in case series contain no data values
    {
        qDebug() << "Data set not found.";
    }

    //the condition below tests the threshold values given and ensures that all three must be within minAxisX and maxAxisX
    else if (vecThresholdValues.x() < minAxisX || vecThresholdValues.y() < minAxisX || vecThresholdValues.z() < minAxisX || vecThresholdValues.x() > maxAxisX || vecThresholdValues.y() > maxAxisX || vecThresholdValues.z() > maxAxisX)
    {
        qDebug() << "One or more of the values given are out of the minimum and maximum range.";
    }

    else
    {
        if ((vecThresholdValues.x() < vecThresholdValues.y()) && (vecThresholdValues.x() < vecThresholdValues.z()))
        {
            leftThresholdValue = vecThresholdValues.x();

            if(vecThresholdValues.y() < vecThresholdValues.z())
            {
                middleThresholdValue = vecThresholdValues.y();
                rightThresholdValue = vecThresholdValues.z();
            }
            else
            {
                middleThresholdValue = vecThresholdValues.z();
                rightThresholdValue = vecThresholdValues.y();
            }
        }
        if ((vecThresholdValues.y() < vecThresholdValues.x()) && (vecThresholdValues.y() < vecThresholdValues.z()))
        {
            leftThresholdValue = vecThresholdValues.y();

            if(vecThresholdValues.x() < vecThresholdValues.z())
            {
                middleThresholdValue = vecThresholdValues.x();
                rightThresholdValue = vecThresholdValues.z();
            }
            else
            {
                middleThresholdValue = vecThresholdValues.z();
                rightThresholdValue = vecThresholdValues.x();
            }
        }
        if ((vecThresholdValues.z() < vecThresholdValues.x()) && (vecThresholdValues.z() < vecThresholdValues.y()))
        {
            leftThresholdValue = vecThresholdValues.z();

            if(vecThresholdValues.x() < vecThresholdValues.y())
            {
                middleThresholdValue = vecThresholdValues.x();
                rightThresholdValue = vecThresholdValues.y();
            }
            else
            {
                middleThresholdValue = vecThresholdValues.y();
                rightThresholdValue = vecThresholdValues.x();
            }
        }
        m_pChart->removeSeries(leftThreshold);
        m_pChart->removeSeries(middleThreshold);
        m_pChart->removeSeries(rightThreshold);

        QPointF leftThresholdPoint;
        QPointF middleThresholdPoint;
        QPointF rightThresholdPoint;

        leftThresholdPoint.setX(leftThresholdValue);
        middleThresholdPoint.setX(middleThresholdValue);
        rightThresholdPoint.setX(rightThresholdValue);

        leftThreshold->clear();
        middleThreshold->clear();
        rightThreshold->clear();

        leftThreshold->append(leftThresholdPoint.x(), 0);
        leftThreshold->append(leftThresholdPoint.x(), maximumFrequency);
        middleThreshold->append(middleThresholdPoint.x(), 0);
        middleThreshold->append(middleThresholdPoint.x(), maximumFrequency);
        rightThreshold->append(rightThresholdPoint.x(), 0);
        rightThreshold->append(rightThresholdPoint.x(), maximumFrequency);

        leftThreshold->setColor("red");
        middleThreshold->setColor("green");
        rightThreshold->setColor("blue");
        leftThreshold->setVisible(true);
        middleThreshold->setVisible(true);
        rightThreshold->setVisible(true);
        m_pChart->addSeries(leftThreshold);
        m_pChart->legend()->markers().at(m_pChart->legend()->markers().size()-1)->setVisible(false);
        m_pChart->addSeries(middleThreshold);
        m_pChart->legend()->markers().at(m_pChart->legend()->markers().size()-1)->setVisible(false);
        m_pChart->addSeries(rightThreshold);
        m_pChart->legend()->markers().at(m_pChart->legend()->markers().size()-1)->setVisible(false);
        m_pChart->createDefaultAxes();
    }
}


//*************************************************************************************************************

const QVector3D& Spline::getThreshold()
{
    QVector3D vec3DReturnVector;

    vec3DReturnVector.setX(leftThreshold->at(0).x());
    vec3DReturnVector.setY(middleThreshold->at(0).x());
    vec3DReturnVector.setZ(rightThreshold->at(0).x());
    return vec3DReturnVector;
}