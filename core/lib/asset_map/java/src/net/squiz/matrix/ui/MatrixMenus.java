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
* $Id: MatrixMenus.java,v 1.1 2005/02/18 05:26:24 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.ui;

import net.squiz.matrix.core.*;
import net.squiz.matrix.assetmap.*;
import net.squiz.matrix.matrixtree.*;

import javax.swing.*;
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
				String linkPath = MatrixToolkit.rawUrlEncode(node.getLinkPath(), true);

				String screenUrl = getScreenUrl(
					asset.getId(),
					assetPath,
					linkPath,
					command
				);
				AssetMap.getURL(screenUrl);
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
	
	/////// USE ME MENU //////
	
	public static JPopupMenu getUseMeMenu(final MatrixTreeNode node) {
		JPopupMenu menu = new JPopupMenu();
		
		ActionListener useMeListener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				Asset asset = node.getAsset();
				String[] info = new String[] {
					asset.getId(),
					asset.getName(),
					node.getURL(),
					node.getLinkid(),
				};
				netscape.javascript.JSObject window = netscape.javascript.JSObject.getWindow(AssetMap.getApplet());
				window.call("asset_finder_done", info);
				MatrixTreeBus.stopAssetFinderMode();
			}
		};
		
		JMenuItem item = new JMenuItem("Use Me");
		item.addActionListener(useMeListener);
		menu.add(item);
		
		return menu;
	}
	
	
	
	////// ADD MENU //////

	public static JPopupMenu getPopupAddMenu(ActionListener listener) {
		return getAddMenu(listener).getPopupMenu();
	}
	
	/**
	 * @return the addmenu
	 */
	public static JMenu getAddMenu(ActionListener listener) {
		
		// if the menu is not null, then we have already created it
		// so just return it
	//	if (menu != null)
	//		return menu;
		
		addMenu = new JMenu("Add New");
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
	private static void addNewItem(MenuElement parent, JMenuItem item, int index, ActionListener listener) {
		if (parent instanceof JPopupMenu)
			((JPopupMenu) parent).add(item, index);
		else 
			((JMenu) parent).add(item, index);
		item.addActionListener(listener);
	}
	
	public static String getTypeCodeFromEvent(ActionEvent evt) {
		JMenuItem item = (JMenuItem) evt.getSource();
		return item.getAccessibleContext().getAccessibleName();
	}
	
	/**
	 * Inner class that handles loading of images for type codes
	 * that the individual menu items represent.
	 * @author Marc McIntyre <mmcintyre@squiz.net>
	 */
	static class ImageLoader implements MenuListener {
	
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
					if (elements[i] instanceof JPopupMenu)
						loadImages(elements[i]);
					else {
						final JMenuItem nextItem = (JMenuItem) elements[i];
						Runnable runner = new Runnable() {
							public void run() {
								Icon icon = type.getIcon();
								nextItem.setIcon(icon);
							}
						};
						SwingUtilities.invokeLater(runner);
					}
				}
			}//end for
		}
		
		/**
		 * Loads the images for the source of the menu when it is selected
		 * @param evt the mouse event
		 */
		public void menuSelected(MenuEvent evt) {
			JMenu menu = (JMenu) evt.getSource();
			loadImages(menu);
		}
		
		public void menuCanceled(MenuEvent evt) {}
		public void menuDeselected(MenuEvent evt) {}

	}//end class ImageLoader
}
