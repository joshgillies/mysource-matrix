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
* $Id: MatrixMenus.java,v 1.13.2.1 2012/11/27 00:00:04 cupreti Exp $
*
*/

package net.squiz.matrix.ui;

import net.squiz.matrix.core.*;
import net.squiz.matrix.debug.*;
import net.squiz.matrix.assetmap.*;
import net.squiz.matrix.matrixtree.*;

import javax.swing.*;
import javax.swing.tree.*;
import java.util.*;
import java.awt.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.net.*;

/**
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixMenus implements MatrixConstants {

	private static JMenu addMenu;
	private static ImageLoader loader = new ImageLoader();
	public static final String DEFAULT_ASSET_ICON = "default_asset.png";

	// cannot instantiate
	private MatrixMenus() {}

	/**
	 * Creates a JPopupMenu with the screens of the specfied asset as menu items.
	 * the action commands of each of the screen items will be obtained by calling
	 * <code>screen.getCodeName()<code> on that particular screen
	 *
	 * @param node the node of the wanted screen
	 * @return the screen menu for the specified node
	 * @see AssetTypeScreen
	 * @see AssetTypeScreen#getCodeName()
	 * @see #getScreenMenu(MatrixTreeNode)
	 * @see #getScreenMenu(MatrixTreeNode, ActionListener)
	 * @see #getPopupScreenMenu(MatrixTreeNode, ActionListener)
	 */
	public static JPopupMenu getPopupScreenMenu(MatrixTreeNode node) {
		return getScreenMenu(node).getPopupMenu();
	}

	/**
	 * Creates a JPopupMenu with the screens of the specfied asset as menu items.
	 * the action commands of each of the screen items will be obtained by calling
	 * <code>screen.getCodeName()<code> on that particular screen
	 *
	 * @param node the node of the wanted screen
	 * @param listener the action listener to apply to the menu
	 * @return the screen menu for the specified node
	 * @see AssetTypeScreen
	 * @see AssetTypeScreen#getCodeName()
	 * @see #getScreenMenu(MatrixTreeNode)
	 * @see #getScreenMenu(MatrixTreeNode, ActionListener)
	 * @see #getPopupScreenMenu(MatrixTreeNode)
	 * @see #getDefaultScreenMenuListener(MatrixTreeNode)
	 */
	public static JPopupMenu getPopupScreenMenu(
		MatrixTreeNode node, ActionListener listener) {
			return getScreenMenu(node, listener).getPopupMenu();
	}

	/**
	 * Creates a JMenu with the screens of the specfied asset as menu items.
	 * the action commands of each of the screen items will be obtained by calling
	 * <code>screen.getCodeName()<code> on that particular screen.
	 *
	 * @param node the node of the wanted screen
	 * @return the screen menu for the specified node
	 * @see AssetTypeScreen
	 * @see AssetTypeScreen#getCodeName()
	 * @see #getScreenMenu(MatrixTreeNode, ActionListener)
	 * @see #getPopupScreenMenu(MatrixTreeNode)
	 * @see #getPopupScreenMenu(MatrixTreeNode, ActionListener)
	 */
	public static JMenu getScreenMenu(MatrixTreeNode node) {
		return getScreenMenu(node, getDefaultScreenMenuListener(node));
	}

	/**
	 * Creates a JMenu with the screens of the specfied asset as menu items.
	 * the action commands of each of the screen items will be obtained by calling
	 * <code>screen.getCodeName()<code> on that particular screen
	 *
	 * @param node the node of the wanted screen
	 * @param listener the action listener to apply to the menu
	 * @return the screen menu for the specified node
	 * @see AssetTypeScreen
	 * @see AssetTypeScreen#getCodeName()
	 * @see #getScreenMenu(MatrixTreeNode)
	 * @see #getPopupScreenMenu(MatrixTreeNode)
	 * @see #getPopupScreenMenu(MatrixTreeNode, ActionListener)
	 * @see #getDefaultScreenMenuListener(MatrixTreeNode)
	 */
	public static JMenu getScreenMenu(MatrixTreeNode node, ActionListener listener) {
		JMenu menu        = new JMenu();
		final Asset asset = node.getAsset();
		AssetType type    = asset.getType();
		Iterator screens  = type.getScreens();

		while (screens.hasNext()) {
			AssetTypeScreen screen = (AssetTypeScreen) screens.next();
			JMenuItem item = new JMenuItem(screen.getScreenName());
			item.setFont(MatrixTreeBus.getActiveTree().getFontInUse());
			item.addActionListener(listener);
			item.setActionCommand(screen.getCodeName());
			menu.add(item);
		}
		return menu;
	}

	/**
	 * Returns a default action listener to be used on screen menus.
	 * When a screen is pressed in the menu, The right main frame of the matrix
	 * system is targeted to display that screen.
	 * @param node the node to target in the matrix system
	 * @return the actionlistener to target matrix screens
	 */
	public static ActionListener getDefaultScreenMenuListener(final MatrixTreeNode node) {
		ActionListener screenListener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				Asset asset = node.getAsset();
				String command = evt.getActionCommand();

				String assetPath = MatrixToolkit.rawUrlEncode(node.getAssetPath(), true);
				String linkPath  = MatrixToolkit.rawUrlEncode(node.getLinkPath(), true);
				String assetid   = MatrixToolkit.rawUrlEncode(asset.getId(), true);

				String screenUrl = getScreenUrl(
					assetid,
					assetPath,
					linkPath,
					command
				);
				try {
					AssetMap.getURL(screenUrl);
				} catch (MalformedURLException mue) {
					Object[] transArgs = {
						mue.getMessage()
					};
					String message = Matrix.translate("asset_map_error_screen_url", transArgs);
					GUIUtilities.error(message, Matrix.translate("asset_map_dialog_title_error"));
					Log.log(message, MatrixMenus.class, mue);
				}
			}
		};
		return screenListener;
	}
	/**
	 * Returns the url for the specified screen name.
	 * @param assetid the assetid if the asset
	 * @param assetPath the asset path to root for this asset
	 * @param linkPath the link path to root for this asset
	 * @param screenName the screen name of the screen
	 */
	private static String getScreenUrl(
		String assetid,
		String assetPath,
		String linkPath,
		String screenName) {
			String url =
				Matrix.getProperty("parameter.url.baseurl") +
				Matrix.getProperty("parameter.backendsuffix") +
				"/?SQ_BACKEND_PAGE=main&backend_section=" +
				"am&am_section=edit_asset&assetid=" + assetid +
				"&sq_asset_path=" + assetPath + "&sq_link_path=" +
				linkPath + "&asset_ei_screen=" + screenName;
			return url;
	}

	// }}}

	// {{{ Use Me Menu

	/**
	 * Returns the use me menu for and adds an action listener to fire for
	 * specified node if the menu is selected.
	 * @param node the node to fire the event for if the menu is selected.
	 * @return the use me menu
	 */
	public static JPopupMenu getUseMeMenu(final MatrixTreeNode node) {
		JPopupMenu menu = new JPopupMenu();

		ActionListener useMeListener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				Asset asset = node.getAsset();
				String[] info = new String[] {
					asset.getId(),
					node.getName(),
					node.getURL(),
					node.getLinkid(),
					asset.getType().toString(),
				};
				netscape.javascript.JSObject window = netscape.javascript.JSObject.getWindow(AssetMap.getApplet());
				window.call("asset_finder_done", info);
				MatrixTreeBus.stopAssetFinderMode();
			}
		};

		JMenuItem item = new JMenuItem(Matrix.translate("asset_map_menu_useme"));
		item.addActionListener(useMeListener);
		item.setFont(MatrixTreeBus.getActiveTree().getFontInUse());
		menu.add(item);

		return menu;
	}

	// }}}

	// {{{ Add Menu

	/**
	 * Returns an actionListener that will start cue mode in the specified tree
	 * when the add menu this action listener is coupled with is triggered.
	 * @param tree the to initiate add mode in
	 * @return the action listener
	 */
	public static ActionListener getMatrixTreeAddMenuListener(final MatrixTree tree) {
		ActionListener addMenuListener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				if (evt.getSource() instanceof JMenuItem) {
					JMenuItem item = (JMenuItem) evt.getSource();

					tree.initiateAddMode(
					new TreePath[] { new TreePath(MatrixMenus.getTypeCodeFromEvent(evt)) },
						item.getLocation()
					);
				}
			}
		};
		return addMenuListener;
	}

	/**
	 * Returns the popup add menu use to add assets to the matrix system. Use this
	 * method in favour of explicitly obtaining the popup menu by using
	 * <code>addMenu.getPopupMenu()</code> as this method handles loading of images
	 * for the add menu.
	 * @param listener the action listener used to trigger events to
	 * @return the popup add menu
	 * @see getAddMenu(ActionListener)
	 */
	public static JPopupMenu getPopupAddMenu(ActionListener listener) {
		JPopupMenu popupAddMenu = getAddMenu(listener).getPopupMenu();
		popupAddMenu.addPopupMenuListener(loader);
		return popupAddMenu;
	}

	/**
	* Returns the add menu to add assets to the matrix system.
	* @param listener the action listener used to trigger events to
	 * @return the addmenu
	 * @see getPopupAddMenu(ActionListener)
	 */
	public static JMenu getAddMenu(ActionListener listener) {

		// if the menu is not null, then we have already created it
		// so just return it
	//	if (menu != null)
	//		return menu;

		addMenu = new JMenu(Matrix.translate("asset_map_menu_add_new"));
		addMenu.addMenuListener(loader);
		Iterator it = AssetManager.getAssetTypes();

		while (it.hasNext()) {
			AssetType type = (AssetType) it.next();

			// if this asset type is not createable by this user
			// then we don't want it in the menu
			if (!(type.isCreatable()))
				continue;

			// get a list of menu paths for this asset type
			// this may be null which indicates that there are no
			// paths for this asset type (it exists in the root "Add" menu)

			String[] paths = type.getMenuPath();
			MenuElement parentMenu = addMenu;

			if (paths.length > 0) {

				boolean found = false;
				String path = null;
				MenuElement[] elements = null;

				for (int i = 0; i < paths.length; i++) {
					path = paths[i];
					found = false;

					// get the current parent menu, and see if we
					// already have this path
					elements = parentMenu.getSubElements();

					if (elements.length > 0 && elements[0] instanceof JPopupMenu)
						elements = elements[0].getSubElements();

					for (int j = 0; j < elements.length; j++) {
						Component component = (Component) elements[j];
						if (component instanceof JMenu) {
							if (component.getAccessibleContext().getAccessibleName().equals(path)) {
								// set the current parent menu to the menu that we just found
								parentMenu = (JMenu) component;
								found = true;
							}
						}
					}//end for

					if (!found) {
						// if we cant find it, create a new JMenu to
						// list this asset type
						JMenu newMenu = new JMenu(path);
						newMenu.getAccessibleContext().setAccessibleName(path);
						newMenu.addMenuListener(loader);

						addNewItem(
							parentMenu,
							newMenu,
							getMenuIndex(path, parentMenu, newMenu.getClass()),
							listener
						);

						// if there are no more paths for this menu,
						// then we want to add this asset type under the
						// newly created menu

						if (i == paths.length - 1) {
							addMenuItem(newMenu, type, listener);
						}
					} else {
						// else we found the JMenu for this path
						// so just add the new asset type under that menu
						addMenuItem(parentMenu, type, listener);
					}//end if not found
				}//end for
			} else {
				// there are no paths for this asset type so just add it in
				// the root "Add" menu
				addMenuItem(addMenu, type, listener);
			}//end if has paths
		}//end while

		return addMenu;
	}

	/**
	 * Returns the menu index for the specified name in the specfied parent menu
	 * @param name the name of the new menu item
	 * @param parentMenu the parent menu where the new menu item will be added
	 * @param cls the class type of the new menu to be added
	 */
	private static int getMenuIndex(String name, MenuElement parentMenu, Class cls){
		int i = 0;
		MenuElement[] components = parentMenu.getSubElements();
		for (i = 0; i < components.length; i++) {

			// because the way that popups work, there may be some menus
			// whose only child is a popupmenu, in that case, we want the children
			// of the popup menu to be processed
			if (components[i] instanceof JPopupMenu)
				return getMenuIndex(name, components[i], cls);

			Component nextMenu = (Component) components[i];
			String otherName = "";

			if (nextMenu.getClass().equals(JMenuItem.class)) {
				String typeCode = nextMenu.getAccessibleContext().getAccessibleName();
				otherName = AssetManager.getAssetType(typeCode).getName();
			} else {
				otherName = nextMenu.getAccessibleContext().getAccessibleName();
			}
			// if its just a single item, then we want it down the bottom
			if (cls.equals(JMenuItem.class) && nextMenu.getClass().equals(JMenu.class))
				continue;
			if (name.compareToIgnoreCase(otherName) < 0)
				return i;
		}
		return i;
	}

	/**
	 * Adds a new item to the specfied menu. The new menu item will have the
	 * name of the specfied asset type <code>type</code>
	 * @param parentMenu the menu where the new JMenuItem will be created
	 * @param type the assetType to reference this JMenuItem to.
	 */
	private static void addMenuItem(MenuElement parentMenu, AssetType type, ActionListener listener) {
		JMenuItem newItem = new JMenuItem(type.getName());
		// newItem.addActionListener(this);
		newItem.getAccessibleContext().setAccessibleName(type.getTypeCode());
		// add single com menu items down the bottom
		int index = getMenuIndex(type.getName(),
			parentMenu,
			newItem.getClass()
		);
		addNewItem(parentMenu, newItem, index, listener);
	}

	/**
	 * Adds the specfied item at the specfied index in the specified parent
	 * @param parent the parent to add the item
	 * @param item the item to add to the parent
	 * @param index the index where the item should be added
	 */
	private static void addNewItem(
		MenuElement parent,
		JMenuItem item,
		int index,
		ActionListener listener) {
			item.setFont(MatrixTreeBus.getActiveTree().getFontInUse());
			if (parent instanceof JPopupMenu)
				((JPopupMenu) parent).add(item, index);
			else
				((JMenu) parent).add(item, index);
			item.addActionListener(listener);
	}

	/**
	 * Returns the type code from that was fired from the add menu for
	 * the specified Event evt.
	 * @return the type code fired from the add menu
	 */
	public static String getTypeCodeFromEvent(ActionEvent evt) {
		JMenuItem item = (JMenuItem) evt.getSource();
		return item.getAccessibleContext().getAccessibleName();
	}

	/**
	 * Inner class that handles loading of images for type codes
	 * that the individual menu items represent.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	static class ImageLoader implements MenuListener, PopupMenuListener {

		/**
		 * loads the images for the specfied parent menu element
		 * @param menu the parent menu to load the images for
		 */
		private void loadImages(MenuElement menu) {
			final MenuElement[] elements = menu.getSubElements();
			for (int i = 0; i < elements.length; i++) {
				if (!(elements[i] instanceof JMenu)) {
					String typeCode = ((Component) elements[i])
						.getAccessibleContext().getAccessibleName();
					final AssetType type = AssetManager.getAssetType(typeCode);

					// because the way that popups work, there may be some menus
					// whose only child is a popupmenu, in that case, we want
					// the children of the popup menu to be processed
					if (elements[i] instanceof JPopupMenu) {
						loadImages(elements[i]);
					} else {
						final JMenuItem nextItem = (JMenuItem) elements[i];
						// Use an initial loadingIcon and fetch the actual icon
						// in a thread
						if (!type.isIconLoaded()) {
							nextItem.setIcon(GUIUtilities.getAssetMapIcon(DEFAULT_ASSET_ICON));
							MatrixSwingWorker worker = new MatrixSwingWorker() {
								public Object construct() {
									Icon icon = type.getIcon();
									nextItem.setIcon(icon);
									return null;
								}
							};
							worker.start();
						} else {
							nextItem.setIcon(type.getIcon());
						}
					}
				}//end if
			}//end for
		}

		/**
		 * Loads the images for the source of the menu when it is selected
		 * @param evt the menu event
		 */
		public void menuSelected(MenuEvent evt) {
			JMenu menu = (JMenu) evt.getSource();
			loadImages(menu);
		}

		/**
		 * Loads the images for the source of the menu when it is selected
		 * @param evt the menu event
		 */
		public void popupMenuWillBecomeVisible(PopupMenuEvent evt) {
			JPopupMenu menu = (JPopupMenu) evt.getSource();
			loadImages(menu);
		}

		public void menuCanceled(MenuEvent evt) {}
		public void menuDeselected(MenuEvent evt) {}
		public void popupMenuCanceled(PopupMenuEvent evt) {}
		public void popupMenuWillBecomeInvisible(PopupMenuEvent evt) {}

	}//end class ImageLoader
}
