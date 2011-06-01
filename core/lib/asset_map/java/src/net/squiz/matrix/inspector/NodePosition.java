/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: NodePosition.java,v 1.2 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

package net.squiz.matrix.inspector;

/**
 * Represents the position of a node in a table.
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class NodePosition {
    public int row;
    public int column;

	/**
	 * Constructs a node position.
	 *
	 * @param row the row of the table that the node is in.
	 * @param column the column of the table that the node is in.
	 */
    public NodePosition(int row, int column) {
		this.row = row;
		this.column = column;
    }

	/**
	 * Returns the row of a particular NodePosition.
	 *
	 * @return the integer value of the row
	 */
	public int getRow() {
		return row;
    }

	/**
	 * Returns the column of a particular NodePosition.
	 *
	 * @return the integer value of the column
	 */
	public int getColumn() {
		return column;
    }
}//end class NodePosition
