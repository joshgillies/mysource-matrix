/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: InspectorTableModel.java,v 1.3 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.inspector;

import javax.swing.table.*;
import javax.swing.*;

public class InspectorTableModel extends DefaultTableModel {

	public InspectorTableModel(int rows, int columns) {
		super(rows, columns);
	}

	public boolean isCellEditable(int row, int column) {
		return false;
	}

	public int getAutoResizeMode() {
		return JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS;

		/*	AUTO_RESIZE_OFF
			AUTO_RESIZE_NEXT_COLUMN
			AUTO_RESIZE_SUBSEQUENT_COLUMNS
			AUTO_RESIZE_LAST_COLUMN
			AUTO_RESIZE_ALL_COLUMNS */
	}
}
