
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
		System.out.println("constructing a matrix split pane divider");
	}

	public void addLeftOneTouchExpandableListener(ActionListener listener) {
		leftButton.addActionListener(listener);
	}

	public void addRightOneTouchExpandableListener(ActionListener listener) {
		rightButton.addActionListener(listener);
	}
}
