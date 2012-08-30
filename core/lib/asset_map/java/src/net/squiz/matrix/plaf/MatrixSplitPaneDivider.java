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
* $Id: MatrixSplitPaneDivider.java,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.plaf;

import javax.swing.plaf.basic.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import net.squiz.matrix.inspector.*;

public class MatrixSplitPaneDivider extends BasicSplitPaneDivider {

	public MatrixSplitPaneDivider(BasicSplitPaneUI ui) {
		super(ui);
	}

	public void addLeftOneTouchExpandableListener(ActionListener listener) {
		leftButton.addActionListener(listener);
	}

	public void addRightOneTouchExpandableListener(ActionListener listener) {
		rightButton.addActionListener(listener);
	}
}
