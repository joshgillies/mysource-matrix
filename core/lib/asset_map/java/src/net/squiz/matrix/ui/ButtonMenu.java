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
* $Id: ButtonMenu.java,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.ui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class ButtonMenu extends JToggleButton {

	private JPopupMenu popup;
	private Icon icon;
	private Icon depressedIcon;
	public ButtonMenu(String text, JPopupMenu popup) {
		super(text);
		addActionListener(buttonAction);
		setPopupMenu(popup);
	}

	public ButtonMenu(Icon icon) {
		super(icon);
		addActionListener(buttonAction);
	}

	public ButtonMenu(Icon icon, Icon depressedIcon) {
		super(icon);
		this.icon = icon;
		this.depressedIcon = depressedIcon;
		addActionListener(buttonAction);
	}

	public void setPopupMenu(JPopupMenu newPopup) {
		if (popup != null) {
			popup.removePopupMenuListener(popupListener);
		}
		popup = newPopup;
		if (popup != null) {
			popup.addPopupMenuListener(popupListener);
		}
	}

	protected void showPopup() {
		if (popup != null) {
			popup.show(ButtonMenu.this, 0, getSize().height);
		}
	}

	protected void hidePopup() {
		if (popup != null) {
			popup.setVisible(false);
		}
	}

	private ActionListener buttonAction = new ActionListener() {
		public void actionPerformed(ActionEvent event) {
			if (isSelected()) {
				showPopup();
			if (depressedIcon != null)
				setIcon(depressedIcon);
			} else {
				hidePopup();
				if (depressedIcon != null)
					setIcon(icon);
			}
		}
	};

	private PopupMenuListener popupListener = new PopupMenuListener() {
		public void popupMenuWillBecomeInvisible(PopupMenuEvent event) {
			if (icon != null)
				setIcon(icon);
			SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					doClick();
				}
			});
		}

		public void popupMenuWillBecomeVisible(PopupMenuEvent event) {}
		public void popupMenuCanceled(PopupMenuEvent event) {}
	};
}
