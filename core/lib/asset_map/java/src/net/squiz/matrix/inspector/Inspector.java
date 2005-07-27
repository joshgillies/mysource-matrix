/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: Inspector.java,v 1.2 2005/07/27 10:45:22 brobertson Exp $
*
*/

/**
 * :tabSize=4:indentSize=4:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 */

package net.squiz.matrix.inspector;

import javax.swing.*;
import net.squiz.matrix.matrixtree.*;
import java.awt.*;
import net.squiz.matrix.core.*;

public class Inspector extends JPanel {

	private InspectorGadget inspector;
	private InspectorNavigator navigator;

	//{{{ Public Methods

	public Inspector(MatrixTree tree) {
		setLayout(new BorderLayout());

		inspector = new InspectorGadget(new InspectorTableModel(0, 4), tree);
		inspector.setDefaultRenderer(inspector.getColumnClass(0), new InspectorCellRenderer());
		inspector.setShowHorizontalLines(false);
		inspector.setShowVerticalLines(false);
		inspector.setRowHeight(50);
		inspector.setRowSelectionAllowed(false);
		inspector.setCellSelectionEnabled(true);
		inspector.addComponentListener(inspector);
		inspector.addTransferListener(inspector);

	//	tree.addNodeDoubleClickedListener(inspector);

		navigator = inspector.getNavigator();
		add(navigator, BorderLayout.NORTH);
	//	pane.setSize(200, 500);
		//pane.setBackground(Color.RED);
		JScrollPane pane = new JScrollPane(inspector);
		add(pane);
	}

	public InspectorGadget getInspectorGadget() {
		return inspector;
	}

	public InspectorNavigator getNavigator() {
		return navigator;
	}

	//}}}

	//{{{ Protected Methods


	//}}}

	//{{{ Package Private Methods


	//}}}

	//{{{ Private Methods


	//}}}

	//{{{ Protected Inner Classes


	//}}}

	//{{{ Inner Classes


	//}}}


}
