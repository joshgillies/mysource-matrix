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
* $Id: InspectorNavigator.java,v 1.1 2005/02/18 05:21:40 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

/**
 * :tabSize=4:indentSize=4:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 */

package net.squiz.matrix.inspector;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.core.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.tree.*;
import java.util.List;
import java.util.ArrayList;

/**
 * The InspectorNavigator class is the navigation panel attached to each
 * InspectorGadget.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
class InspectorNavigator extends JPanel {

	private JButton backBtn;
	private JButton forwardBtn;
	private JButton showPathBtn;
	private InspectorGadget inspector;
	private TreePath current;
	private List backPath = new ArrayList();
	private List forwardPath = new ArrayList();

	//{{{ Public Methods

	/**
	 * Returns an InspectorNavigator.
	 * @param inspector the InspectorGadget component this panel navigates
	 * @param current the path that the associated InspectorGadget is currently
	 * displaying
	 */
	public InspectorNavigator(InspectorGadget inspector/*, TreePath current*/) {
		this.inspector = inspector;
		setBackground(UIManager.getColor("InspectorNavigator.background"));
		setLayout(new FlowLayout(FlowLayout.LEADING));
		init();
	}

	/**
	 * Sets the current TreePath of the Inspector
	 */
	public void setCurrentPath(TreePath currentPath) {
		current = currentPath;
	}

	/**
	 * Keeps a record of which TreePaths have been viewed in the InspectorGadget
	 * by adding the currently viewed TreePath to the back history, changing the
	 * current path to the new path, and clearing the forward history.
	 */
	public void setBackPath(TreePath path) {
		if (current != null) backPath.add(current);
		current = path;
		forwardPath.clear();
		updateBackForward();
	}

	/**
	 * Disables and enables the various buttons in the InspectorNavigator panel
	 * according to the available back and forward history. Also sets the tooltip
	 * for the path button.
	 */
	public void updateBackForward() {
		if (backPath.size() == 0) backBtn.setEnabled(false);
		else backBtn.setEnabled(true);
		if (forwardPath.size() == 0) forwardBtn.setEnabled(false);
		else forwardBtn.setEnabled(true);
		if (current.getPathCount() < 2)
			showPathBtn.setEnabled(false);
		else showPathBtn.setEnabled(true);

		Object[] treePathObjects = new Object[current.getPathCount() - 1];
		for (int k = 1; k < current.getPathCount(); k++) {
		treePathObjects[k-1] = current.getPathComponent(k);
		}

		showPathBtn.setToolTipText(convertPathToLocation(new TreePath(treePathObjects)));
	}

	/**
	 * Disables all the buttons on the navigator panel.
	 */
	public void disablePanel() {
		forwardBtn.setEnabled(false);
		backBtn.setEnabled(false);
		showPathBtn.setEnabled(false);
	}

	//}}}

	//{{{ Protected Methods
	//}}}

	//{{{ Package Private Methods
	//}}}

	//{{{ Private Methods

	/**
	 * Creates the physical panel and its buttons
	 */
	private void init() {
		setSize(240, 40);
		setBackground(UIManager.getColor("InspectorNavigator.background"));

		ActionListener btnListener = new ButtonActionListener();

		backBtn     = createButton("back", btnListener, "Back");
		forwardBtn  = createButton("forward", btnListener, "Forward");
		showPathBtn = createButton("show_path", btnListener, "");

		add(backBtn);
		add(forwardBtn);
		add(showPathBtn);

		disablePanel();
	}

	/**
	 * Creates a button and applies the
	 * adds the actionListener to it. The button
	 * will have the supplied icon name from the lib/web
	 * directory of the matrix install, and will display
	 * the supplied tooltip when hovered over.
	 *
	 * @param iconName the name of the icon including the extension
	 * @param listener the ActionListener to add
	 * @param toolTipText the tooltip text to display
	 *
	 * @return the newly created button
	 */
	private JButton createButton(
			String iconName,
			ActionListener listener,
			String toolTipText) {

		Icon icon = GUIUtilities.getAssetMapIcon(iconName + ".png");
		Icon disabledIcon = GUIUtilities.getAssetMapIcon(iconName + "_disabled.png");
		Icon pressedIcon = GUIUtilities.getAssetMapIcon(iconName + "_on.png");

		JButton button = new JButton(icon);
		button.setDisabledIcon(disabledIcon);
		button.setPressedIcon(pressedIcon);
		button.setBorderPainted(false);

		button.setPreferredSize(new Dimension(icon.getIconWidth(), icon.getIconHeight()));

		button.addActionListener(listener);
		button.setToolTipText(toolTipText);

		return button;
	}

	/**
	 * Populates the InspectorGadget with the last path item in its back
	 * history.
	 */
	private void navigateBack() {
		int backLastIndex = backPath.size() - 1;
		TreePath oldPath = (TreePath) backPath.get(backLastIndex);
		inspector.populateInspector(oldPath);
		forwardPath.add(current);
		current = oldPath;
		backPath.remove(backLastIndex);
		updateBackForward();
	}

	/**
	 * Populates the InspectorGadget with the first path item in its forward
	 * history.
	 */
	private void navigateNext() {
		int forwardLastIndex = forwardPath.size() - 1;
		TreePath oldPath = (TreePath) forwardPath.get(forwardLastIndex);
		inspector.populateInspector(oldPath);
		backPath.add(current);
		current = oldPath;
		forwardPath.remove(forwardLastIndex);
		updateBackForward();
	}

	/**
	 * Creates a current path lineage.
	 */
	private String convertPathToLocation(TreePath path) {
		Object[] nodes = path.getPath();
		String pathStr = "";

		// MM: start at one so that we dont have the root folder
		// NdV: BUT WE WANT THE ROOT FOLDER!
		for (int i = 0; i < nodes.length; i++) {
			pathStr = pathStr + ((MatrixTreeNode) nodes[i]).getAsset().getName();
			if (i != nodes.length - 1)
				pathStr = pathStr + " > ";
		}
		return pathStr;
	}

	//}}}

	//{{{ Protected Inner Classes
	//}}}

	//{{{ Inner Classes

	/**
	 * The class ButtonActionListener defines the various actions which occur
	 * when it recieves action events from the buttons on an InspectorNavigator.
	 *
	 * @author Nathan de Vries <ndvries@squiz.net>
	 */
	class ButtonActionListener implements ActionListener {

		/**
		 * Invoked when an action occurs.
		 * @param e A semantic event which indicates that a component-defined
		 * action occured.
		 */
		public void actionPerformed(ActionEvent evt) {
			Object source = evt.getSource();

			if (source == backBtn) navigateBack();
			else if (source == forwardBtn) navigateNext();
			else if (source == showPathBtn) {

				JPopupMenu menu = new JPopupMenu();

				menu.add( 	new AbstractAction("My Matrix System") {
								public void actionPerformed(ActionEvent e) {
									inspector.populateInspector(inspector.getTree().getPathToRoot((MatrixTreeNode)AssetManager.getRootFolderNode()));
									setBackPath(inspector.getTree().getPathToRoot((MatrixTreeNode)AssetManager.getRootFolderNode()));
								}
							});

				//if (current.getPathCount() > 1) menu.addSeparator();

				for (int i = 1; i < current.getPathCount(); i++) {
					final TreePath path = inspector.getTree().getPathToRoot((MatrixTreeNode)current.getPathComponent(i));

					Object[] treePathObjects = new Object[path.getPathCount() - 1];
					for (int k = 1; k < path.getPathCount(); k++) {
						treePathObjects[k-1] = path.getPathComponent(k);
					}

					String itemPath = convertPathToLocation(new TreePath(treePathObjects));

					// MM: is there a good reason why we are using AbstractAction here?
					// They are more expensive than ActionListeners and are generally only
					// needed if you are going to reuse them in menus and Keystokes or you
					// are going to make good use of the isEnabled() / setEnabled() accessor methods etc.
					// Using an AbstractAction will still create a JMenuItem internally in JMenu anyway.
					// it would probably be better to create a menuActionListener outside of the for loop
					// @see http://beta.squiz.net/docs/java_sdk/docs/api/javax/swing/Action.html

					menu.add( 	new AbstractAction(itemPath, ((MatrixTreeNode)path.getLastPathComponent()).getAsset().getType().getIcon()) {
									public void actionPerformed(ActionEvent e) {
										inspector.populateInspector(path);
										setBackPath(path);
									}
								});

				}
				menu.show(InspectorNavigator.this, showPathBtn.getX() + 8, showPathBtn.getY() + showPathBtn.getHeight() + 1);

				System.out.println("current is: " + current);
				System.out.println("backPath is: " + backPath);
				System.out.println("forwardPath is: " + forwardPath);
				System.out.println("There are " + backPath.size() + " backPath items");
				System.out.println("There are " + forwardPath.size() + " forwardPath items");
			}
		}
	}//end class ButtonActionListener

	//}}}

}//end class InspectorNavigator
