/*
  ==============================================================================

    TracerPoint.cpp
    Created: 27 Jul 2020 12:31:18pm
    Author:  brady

  ==============================================================================
*/

#include "TracerPoint.h"
#include "Hexagon.h"

bool TracerPoint::operator==(TracerPoint const& point)
{
    if (point.vertex == vertex && hexPos.col == point.hexPos.col && hexPos.row == point.hexPos.row)
    {
        return true;
    }
    return false;
}

bool TracerPoint::operator!=(TracerPoint const& point)
{
    if (point.vertex != vertex || hexPos.col != point.hexPos.col || hexPos.row != point.hexPos.row)
    {
        return true;
    }
    return false;
}

void TracerPoint::initLinePos()
{
    if (vertex < 2)
    {
        /* General case for vertices 0, 1 */
        pos.col = hexPos.col + 1;
        if (hexPos.col % 2 == 0)
        {
            pos.row = (2 * hexPos.row) + vertex + 1;
        }
        else
        {
            pos.row = (2 * hexPos.row) + vertex;
        }
    }
    else {
        /* Edge cases */
        if (vertex > 2)
        {
            pos.col = hexPos.col;
        }
        else
        {
            pos.col = hexPos.col + 1;
        }

        if (hexPos.row == 0)
        {
            if (vertex == 5 && hexPos.col == 0)
            {
                /* top of first col */
                pos.row = 1;
            }
            else
            {
                /* Odd column top row */
                pos.row = 0;
            }
        }
        else if (hexPos.col % 2)
        {
            /* Odd column bottom row */
            if (vertex > 3)
            {
                pos.row = (NUM_ROWS * 2) - 1;
            }
            else
            {
                pos.row = (NUM_ROWS * 2);
            }
        }
        else if (hexPos.col == 0)
        {
            if (hexPos.row == 0 && vertex == 5)
            {
                /* Top left tile */
                pos.row = 1;
            }
            else
            {
                /* First column */
                pos.row = (2 * hexPos.row) + (5 - vertex) + 1;
            }
        }
        else if (hexPos.col == NUM_COLS - 1)
        {
            /* Bottom right tile */
            pos.row = (NUM_ROWS * 2) - 1;
        }
        else
        {
            // Logic must be wrong, edge case not caught */
            jassert(false);
        }
    }
    positionChanged();
}

/* updates hex position and intersection type */
void TracerPoint::positionChanged()
{
    /* Reset intersection type */
    intType = INVALID;

    /* Assuming new line pos has been set, need to update hexPos to match */
    /* Refresh column */
    if (vertex < 2)
    {
        /* General case for vertices 0, 1 */
        hexPos.col = pos.col - 1;
        if (hexPos.col % 2 == 0)
        {
            hexPos.row = (pos.row - vertex - 1) / 2;
        }
        else
        {
            hexPos.row = (pos.row - vertex) / 2;
        }

        /* Rightmost col at vertex 1 are up downs */
        if (pos.col == NUM_COLS && vertex == 1)
        {
            intType = UP_DOWN;
        }
        else if ((pos.row == 0 && hexPos.col % 2 == 1) || (pos.row == 1 && pos.col == NUM_COLS))
        {
            /* top of last col */
            intType = LEFT_RIGHT;
        }
        /* Set intersection type with a clever xor */
        else if ((pos.row % 2 == pos.col % 2) || (pos.row == 1 && pos.col == 0 && vertex == 0))
        {
            /* Odd row, odd col or even row, even col */
            /* vertex 0 of top of first col */
            intType = LEFT_T;
        }
        else
        {
            /* Odd row, even col or even row, odd col */
            intType = RIGHT_T;
        }
    }
    else
    {
        /* Edge cases */
        if (vertex > 2)
        {
            hexPos.col = pos.col;
        }
        else
        {
            /* vertex = 2 */
            hexPos.col = pos.col - 1;
        }

        if (pos.row == 0)
        {
            /* Odd column top row vertex 5*/
            hexPos.row = 0;
            intType = LEFT_RIGHT;
        }
        else if (pos.row == (NUM_ROWS * 2) || (pos.row == ((NUM_ROWS * 2) - 1) && vertex == 4))
        {
            /* Odd column bottom row */
            hexPos.row = NUM_ROWS - 1;
            if (vertex == 4)
            {
                intType = LEFT_T;
            }
            else
            {
                intType = LEFT_RIGHT;
            }
        }
        else if (pos.col == 0)
        {
            if (pos.row == 1 && vertex == 5)
            {
                /* Top left tile */
                hexPos.row = 0;
                intType = LEFT_RIGHT;
            }
            else
            {
                /* First column */
                hexPos.row = (pos.row - 1 - (5 - vertex)) / 2;
                if (vertex == 4)
                {
                    intType = UP_DOWN;
                }
                else if (vertex == 3 && pos.row == 2 * NUM_ROWS - 1)
                {
                    /* bottom left hex */
                    intType = LEFT_RIGHT;
                }
                else
                {
                    intType = RIGHT_T;
                }
            }
        }
        else if (hexPos.col == NUM_COLS - 1)
        {
            /* Bottom right tile, vertex = 2 */
            /* DBG("\n" << "BOTTOM RIGHT"); */
            hexPos.row = NUM_ROWS - 2;
            intType = LEFT_RIGHT;
        }
    }

    if (intType == INVALID)
    {
        if (vertex % 2)
        {
            intType = LEFT_T;
        }
        else
        {
            intType = RIGHT_T;
        }
    }
}

Array<TracerPoint::Direction> TracerPoint::getMoves()
{
    Array<Direction> moves;
    switch (intType)
    {
    case LEFT_T:
    {
        moves.add(LEFT);
        moves.add(UP);
        moves.add(DOWN);
        break;
    }
    case RIGHT_T:
    {
        moves.add(RIGHT);
        moves.add(UP);
        moves.add(DOWN);
        break;
    }
    case LEFT_RIGHT:
    {
        moves.add(LEFT);
        moves.add(RIGHT);
        break;
    }
    case UP_DOWN:
    {
        moves.add(UP);
        moves.add(DOWN);
        break;
    }
    default:
    {
        jassert(false);
    }
    }
    return moves;
}

/*
TracerPoint TracerPoint::getPathEnd(Array<Direction> path)
{
    TracerPoint curPoint = *this;
    for (Direction dir : path)
    {
        curPoint.move(dir);
    }
    return curPoint;
}
*/

void TracerPoint::move(Direction dir)
{
    switch (dir)
    {
    case UP:
    {
        pos.row--;
        if (intType == UP_DOWN)
        {
            if (vertex == 4 && pos.row == 1)
            {
                /* Top left tile (col 0, row 1) */
                vertex = 5;
            }
            else if (vertex > 1) /* vertex is 3 or 4 */
            {
                /* Left col up downs */
                /* vertex = (vertex == 3) ? 4 : 3; */
                vertex = 3;
            }
            else
            {
                /* Right col up downs */
                vertex = 0;
            }
        }
        else
        {
            if (vertex == 4)
            {
                vertex = 1;
            }
            else if (vertex == 0 && pos.row == 0)
            {
                vertex = 5;
            }

            else if (vertex == 3 && intType == RIGHT_T)
            {
                /* column 0 */
                vertex = 4;
            }
            else
            {
                /* bottom left right */
                vertex = !vertex;
            }
        }
        break;
    }
    case DOWN:
    {
        pos.row++;
        if (intType == UP_DOWN)
        {
            if (vertex > 1)
            {
                /* Left col up downs */
                vertex = (vertex == 3) ? 4 : 3;
            }
            else if (pos.row == (2 * NUM_ROWS) - 1)
            {
                /* Bottom right tile */
                vertex = 2;
            }
            else
            {
                /* Right col up downs */
                vertex = 0;
            }
        }
        else
        {
            if (vertex == 3)
            {
                /* col 0 RIGHT_T */
                vertex = 4;
            }
            else if (vertex == 4)
            {
                /* bottom left LEFT_T */
                vertex = 3;
            }
            else if (pos.row == 2 * NUM_ROWS)
            {
                /* down to last row */
                if (intType == 1)
                {
                    vertex = 2;
                }
                else
                {
                    vertex = 4;
                }
            }
            else if (pos.row == 2 * NUM_ROWS - 1)
            {
                /* down to 2nd to last row */
                if (vertex == 1)
                {
                    if (pos.col == NUM_COLS)
                    {
                        /* bottom right hex */
                        vertex = 2;
                    }
                    else
                    {
                        vertex = 4;
                    }
                }
                else if (vertex == 4)
                {
                    /* bottom of first line col*/
                    vertex = 3;
                }
                else
                {
                    vertex = 1;
                }
            }

            else
            {
                vertex = !vertex;
            }

        }
        break;
    }
    case LEFT:
    {
        if (intType == LEFT_RIGHT)
        {
            if (vertex == 0 && hexPos.col % 2 == 1)
            {
                /* Odd top right side */
                vertex = 5;
                pos.col--;
            }
            else if (vertex == 5)
            {
                /* Odd top left side */
                vertex = (pos.col == 0) ? 4 : 0;
                pos.row++;
            }
            else if (vertex == 2)
            {
                pos.col--;
                vertex = (pos.row == (2 * NUM_ROWS)) ? 3 : 1;
            }
            else if (vertex == 0 && pos.row == 1)
            {
                vertex = 1;
                pos.col--;
            }
            else /* vertex == 3 */
            {
                pos.row--;
                vertex = 4;
            }
        }
        else
        {
            /* jassert(vertex < 2); */
            if (vertex == 4)
            {
                if (pos.col == 1)
                {
                    /* bottom of first col*/
                    vertex = 3;
                }
                else
                {
                    vertex = 1;
                }
                pos.col--;
            }
            else
            {
                pos.col--;

                if (pos.col == 0)
                {
                    if (pos.row == 1)
                    {
                        /* top of first col */
                        vertex = 5;
                    }
                    else
                    {
                        vertex = 3;
                    }
                }
                else
                {
                    vertex = 1;
                }
            }
        }
        break;
    }
    case RIGHT:
    {
        if (intType == LEFT_RIGHT)
        {
            if (vertex == 0)
            {
                /* Odd top left side */
                vertex = 1;
                pos.row++;
            }
            else if (vertex == 5)
            {
                /* Odd top left side */
                vertex = 0;
                pos.col++;
            }
            else if (vertex == 2)
            {
                vertex = 1;
                pos.row--;
            }
            else if (vertex == 3)
            {
                if (pos.col == 0)
                {
                    /* bottom of first col */
                    vertex = 4;
                }
                else
                {
                    vertex = 2;
                }
                pos.col++;
            }
            else /* vertex == 3 */
            {
                pos.col++;
                vertex = 2;
            }
        }
        else
        {
            if (vertex == 3)
            {
                /* RIGHT_T col 0 */
                vertex = 0;
                pos.col++;
            }

            else
            {
                pos.col++;
                if (pos.row == (2 * NUM_ROWS) - 1)
                {
                    if (pos.col == NUM_COLS)
                    {
                        /* bottom right hex*/
                        vertex = 2;
                    }
                    else
                    {
                        vertex = 4;
                    }

                }
                else
                {
                    vertex = 0;
                }
            }
        }
        break;
    }
    default:
    {
        jassert(false);
    }
    }

    positionChanged();
}
