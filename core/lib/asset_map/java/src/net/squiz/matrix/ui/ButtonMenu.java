
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
			System.out.println("in the action listener");
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
					System.out.println("doing click");
					doClick();
				}
			});
		}
	
		public void popupMenuWillBecomeVisible(PopupMenuEvent event) {}
		public void popupMenuCanceled(PopupMenuEvent event) {}
	};
}
