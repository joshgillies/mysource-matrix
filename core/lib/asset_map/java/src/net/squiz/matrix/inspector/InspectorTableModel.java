
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
