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
* $Id: Inspector.java,v 1.3 2006/12/05 05:26:36 bcaldwell Exp $
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
