/*
  ==============================================================================

    HexGrid.cpp
    Created: 14 Jun 2020 3:32:44pm
    Author:  brady

  ==============================================================================
*/

#include <JuceHeader.h>
#include "HexGrid.h"

#define PADDING 15

HexGrid::HexGrid()
{
    setSize(200, 300);
    for (int i = 0; i < NUM_COLS; i++)
    {
        int numRows = (i % 2 == 0) ? (NUM_ROWS - 1) : NUM_ROWS;

        for (int j = 0; j < numRows; j++)
        {
            /* Yes this is bad for locality, but it's the easiest to traverse with flat hexagons */
            m_hexArray[i][j].setTile(NoteUtils::hexagons[j][i]);
            m_hexArray[i][j].setPosition(j, i);
            m_hexArray[i][j].addMouseListener(this, false);
            addAndMakeVisible(m_hexArray[i][j]);
        }
    }
    
    /* for (int i = 0; i < 2; i++)
    {
        m_tracers.add(new Tracer());
        m_tracers[i]->setSize(15, 15);
        m_tracers[i]->position = TracerPoint(5, 7, 0);
        addAndMakeVisible(m_tracers[i]);
    } */

    m_timerCount = 0;    
}

void HexGrid::addPathClicked(bool isAdding)
{
    m_canDrag = isAdding;
}

void HexGrid::mouseMove(const MouseEvent& e)
{
    if (m_canDrag)
    {
        if (m_isHexMode)
        {
            Hexagon* hex = (Hexagon*)e.eventComponent;
            if (hex->isPointInside(e.getPosition().toFloat()) && hex != m_hoveringOverHex)
            {
                if (m_hoveringOverHex != nullptr)
                {
                    m_hoveringOverHex->setSelected(false);
                }
                m_hoveringOverHex = hex;
                hex->setSelected(true);
                DBG("Pos: row: " << hex->getRow() << ", col: " << hex->getCol());
            }
        }
        else
        {
            TracerPoint hoverTracer = getNearestCoord(e.getMouseDownPosition());
            m_hoveringOverPoint = m_hexArray[hoverTracer.hexPos.col][hoverTracer.hexPos.row].getVertex(hoverTracer.vertex);
        }

        repaint();
    }
}

void HexGrid::mouseExit(const MouseEvent& event)
{
    if (m_canDrag)
    {
        if (m_isHexMode && m_hoveringOverHex != nullptr)
        {
            m_hoveringOverHex->setSelected(false);
            m_hoveringOverHex = nullptr;
        }
        else
        {
            /* Move offscreen if mouse is off of grid */
            m_hoveringOverPoint = Point<float>(-10, -10);
            repaint();
        }

    }
}

void HexGrid::mouseDrag(const MouseEvent& event)
{

}

void HexGrid::mouseDown(const MouseEvent& event)
{
    if (m_canDrag)
    {
        m_isDragging = true;
        TracerPoint tPoint = getNearestCoord(event.getMouseDownPosition());
    }
}

void HexGrid::mouseUp(const MouseEvent& event)
{
    if (m_isDragging)
    {
        m_isDragging = false;
    }
}

TracerPoint HexGrid::getNearestCoord(Point<int> dragStart)
{
    TracerPoint::coord hexPosition;
    hexPosition.col = 1; hexPosition.row = 1;
    int xVariance = m_hexArray[0][0].getWidth() / 2;
    /* Set x origin */
    if (dragStart.x <= xVariance) hexPosition.col = 0; // Check first line col
    for (int i = 0; i < NUM_COLS; i++)
    {
        int curX = m_hexArray[i][0].getVertex(0).x;
        if ((dragStart.x >= curX - xVariance ) && (dragStart.x <= curX + xVariance))
        {
            hexPosition.col = i;
        }
    }
    hexPosition.row = 1;
    /*for (int i = 0; i < NUM_ROWS; i++)
    {

    } */
    return TracerPoint(hexPosition.row, hexPosition.col, 0);
}

void HexGrid::moveTracers(int duration)
{
    if (!m_animator.isAnimating())
    {
        m_timerCount++;
        for (int i = 0; i < m_tracers.size(); i++)
        {
            Rectangle<int> center = m_tracers[i]->getBounds();
            moveTracerRandom(m_tracers[i]);
            center.setCentre(getTracerPosition(m_tracers[i]->position).toInt());
            m_animator.animateComponent(m_tracers[i], center, 1, duration - 10, true, 0.3, 0.3);
            repaint();
        }
    }
}

HexGrid::~HexGrid()
{
}

void HexGrid::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background

    if (m_canDrag)
    {
        /* If user can drag, draw a fake tracer on the nearest intersection */
        g.setColour(Colours::aqua);
        Rectangle<float> circle(0, 0, 15, 15);
        circle.setCentre(m_hoveringOverPoint.x, m_hoveringOverPoint.y);
        g.drawEllipse(circle, 2);
    }
}

void HexGrid::resized()
{
    /* Position hexagons */
    float hexHeight = (getHeight()- PADDING) / NUM_ROWS;
    float hexWidth = hexHeight * HEX_W_TO_H_RATIO;

    float curX = PADDING;
    float curY = PADDING;
    float yOffset = hexHeight / 2;
    int numRows = NUM_ROWS;
    for (int i = 0; i < NUM_COLS; i++)
    {
        if (i > 0)
        {
            // Set x to previous column's top right vertex coordinate
            curX = m_hexArray[i-1][0].getVertex(0).x;
        }
        curY = (i % 2 == 0) ? yOffset : 0;
        numRows = (i % 2 == 0) ? (NUM_ROWS - 1) : NUM_ROWS;

        for (int j = 0; j < numRows; j++)
        {
            /* Yes this is bad for locality, but it's the easiest to traverse with flat hexagons */
            m_hexArray[i][j].setBounds(curX, curY, hexWidth, hexHeight);
            curY += hexHeight;
        }
    }

    for (int i = 0; i < m_tracers.size(); i++)
    {
        Point<float> tracerPos = getTracerPosition(m_tracers[i]->position);
        m_tracers[i]->setCentrePosition(tracerPos.toInt());
    }
}

Array<Hexagon*> HexGrid::getNotesToPlay()
{
    Array<Hexagon*> notes;
    for (int i = 0; i < m_tracers.size(); i++)
    {
        Array <Hexagon*> tracerHex = getNotes(m_tracers[i]);
        for (int j = 0; j < tracerHex.size(); j++)
        {
            notes.add(tracerHex[j]);
        }

    }
    return notes;
}

Point<float> HexGrid::getTracerPosition(TracerPoint point)
{
    return m_hexArray[point.hexPos.col][point.hexPos.row].getVertex(point.vertex);
}

void HexGrid::moveTracerRandom(Tracer *tracer)
{
    Array<TracerPoint::Direction> possibleDirs = tracer->position.getMoves();
    int index = Random::getSystemRandom().nextInt(possibleDirs.size());
    return tracer->position.move(possibleDirs[index]);
}

/* returns array of HexTile structs the tracer is currently touching */
Array <Hexagon*> HexGrid::getNotes(Tracer *tracer)
{
   /* DBG("tracer line row: " << tracer->position.pos.row);
    DBG("tracer line col: " << tracer->position.pos.col);
    DBG("tracer intType: " << tracer->position.intType); */

    Array <Hexagon*> notes;
    
    /* 8 rows, 15 cols */
    
    switch(tracer->position.intType)
    {
        case TracerPoint::LEFT_T:
        {
            /* internal: one left, two right */
            if (tracer->position.pos.col % 2 == 1)
            {
                if (tracer->position.pos.row == 1)
                {
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][0]);
                    notes.add(&m_hexArray[tracer->position.pos.col][0]);
                }
                else if (tracer->position.pos.col == NUM_COLS)
                {
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2]);
                }
                else if (tracer->position.pos.row == NUM_ROWS * 2 - 1)
                {
                    /* hex row 7 vertex 5 */
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2]);
                }
                else
                {
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2]);
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2]);
                }
            }
            else
            {
                notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
                notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2]);
                notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2 - 1]);
            }

            break;
        }
        case TracerPoint::RIGHT_T:
        {
            if (tracer->position.pos.col % 2 == 1)
            {
                notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
                notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2 - 1]);
                notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2]);
            }
            else
            {
                if (tracer->position.pos.col == 0)
                {
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2 - 1]);
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2]);
                }
                else if (tracer->position.pos.row == 1)
                {
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][0]);
                    notes.add(&m_hexArray[tracer->position.pos.col][0]);
                }
                else if (tracer->position.pos.row == NUM_ROWS * 2 - 1)
                {
                    /* hex row 7, odd col */
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2]);
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2 - 1]);
                }
                else
                {
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2]);
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2]);
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2 - 1]);
                }
            }
            break;
        }
        case TracerPoint::LEFT_RIGHT:
        {
            /* only one hex */
            if ( tracer->position.pos.row == 0)
            {
                /* top of odd cols */
                if (tracer->position.vertex == 5)
                {
                    notes.add(&m_hexArray[tracer->position.pos.col][0]);
                }
                else
                {
                    /* vertex = 0 */
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row]);
                }
            }
            else if (tracer->position.pos.row / 2 == NUM_ROWS)
            {
                /* hex in row 7 vertex 2 or 3 */
                if (tracer->position.vertex == 2)
                {
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
                }
                else
                {
                    notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2 - 1]);
                }
            }
            else if (tracer->position.pos.row == NUM_ROWS * 2 - 1)
            {
                /* bottom of first and last col */
                if (tracer->position.pos.col == 0)
                {
                    /* bottom of col 0 vertex 3 */
                     notes.add(&m_hexArray[0][tracer->position.pos.row / 2 - 1]);
                }
                else if (tracer->position.pos.col == NUM_COLS)
                {
                    /* bottom of last col vertex 2 */
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
                }
            }
            else
            {
                /* top of first and last col */
                if (tracer->position.pos.col == 0)
                {
                    /* top of col 0 vertex 5 */
                     notes.add(&m_hexArray[0][0]);
                }
                else if (tracer->position.pos.col == NUM_COLS)
                {
                    /* top of last col vertex 0 */
                    notes.add(&m_hexArray[tracer->position.pos.col - 1][0]);
                }
            }
            break;
        }
        case TracerPoint::UP_DOWN:
        {
            if (tracer->position.pos.col == 0)
            {
                notes.add(&m_hexArray[tracer->position.pos.col][tracer->position.pos.row / 2 - 1]);
            }
            else{
                /* last col vertex 1*/
                notes.add(&m_hexArray[tracer->position.pos.col - 1][tracer->position.pos.row / 2 - 1]);
            }
            break;
        }
        default:
        {
            jassert(false);
        }
    }
    
    return notes;
}
