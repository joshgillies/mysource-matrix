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
* $Id: InspectorNavigator.java,v 1.6 2012/08/30 01:09:20 ewang Exp $
*
*/

/**
 * :tabSize=4:indentSize=4:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 */

package net.squiz.matrix.inspector;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
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
	private ButtonMenu showPathBtn;
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

		showPathBtn.setToolTipText(convertPathToString(new TreePath(treePathObjects)));
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

		backBtn     = (JButton) createButton("back", btnListener, "Back", false);
		forwardBtn  = (JButton) createButton("forward", btnListener, "Forward", false);
		showPathBtn = (ButtonMenu) createButton("show_path", btnListener, "Show Path", true);

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
	private AbstractButton createButton(
			String iconName,
			ActionListener listener,
			String toolTipText,
			boolean buttonMenu) {

		Icon icon = GUIUtilities.getAssetMapIcon(iconName + ".png");
		Icon disabledIcon = GUIUtilities.getAssetMapIcon(iconName + "_disabled.png");
		Icon pressedIcon = GUIUtilities.getAssetMapIcon(iconName + "_on.png");

		AbstractButton button = null;
		if (buttonMenu)
			button = new ButtonMenu(icon, pressedIcon);
		else
			button = new JButton(icon);

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
	private String convertPathToString(TreePath path) {
		return convertPathToString(path.getPath());
	}

	private String convertPathToString(Object[] nodes) {
		String pathStr = "";

		for (int i = 0; i < nodes.length; i++) {
			pathStr = pathStr + ((MatrixTreeNode) nodes[i]).getName();
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

		private boolean isVisible = false;
		private JPopupMenu menu;
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
				showPathBtn.setPopupMenu(getMenu());
			}
		}

		private JPopupMenu getMenu() {
			JPopupMenu menu = new JPopupMenu();

			ActionListener matrixListener = new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					MatrixTreeNode rootNode
						= (MatrixTreeNode) AssetManager.getRootFolderNode();
					TreePath rootPath = inspector.getTree().getPathToRoot(rootNode);
					inspector.populateInspector(rootPath);
					setBackPath(rootPath);
				}
			};

			JMenuItem matrixItem = new JMenuItem("My Matrix System");
			matrixItem.addActionListener(matrixListener);
			menu.add(matrixItem);

			if (current.getPathCount() > 1) menu.addSeparator();

			for (int i = 1; i < current.getPathCount(); i++) {
				final TreePath path = inspector.getTree().getPathToRoot(
					(MatrixTreeNode) current.getPathComponent(i));

				Object[] treePathObjects = new Object[path.getPathCount() - 1];
				for (int k = 1; k < path.getPathCount(); k++)
					treePathObjects[k - 1] = path.getPathComponent(k);

				String pathStr = convertPathToString(treePathObjects);
				ActionListener pathListener = new ActionListener() {
					public void actionPerformed(ActionEvent evt) {
						inspector.populateInspector(path);
						setBackPath(path);
					}
				};

				MatrixTreeNode node = (MatrixTreeNode) path.getLastPathComponent();
				Icon icon = node.getAsset().getType().getIcon();
				JMenuItem pathItem = new JMenuItem(pathStr);
				pathItem.addActionListener(pathListener);
				pathItem.setIcon(icon);

				menu.add(pathItem);
			}

			return menu;
		}

	}//end class ButtonActionListener

	//}}}

}//end class InspectorNavigator
