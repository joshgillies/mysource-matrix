package net.squiz.matrix.complexassetmap;

import net.squiz.matrix.assetmap.*;
import javax.swing.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.awt.*;
import java.util.*;

/**
* A panel for the components to add Assets, Refresh the tree, help etc
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*/
public class AssetMapMenuPanel extends JPanel 
	implements MenuListener, ActionListener {

	/** the menu bar used to hold the asset menu */
	private JMenuBar menuBar;
	
	/** the complex asset tree instance */
	private ComplexAssetTree tree;
	
	/** the toolbar used for the buttons */
	private JToolBar toolBar;
	
	/** The menu for use in the add menu */
	public static final Font menuFont = new Font("menu_font", Font.PLAIN, 9);
	
	/** the bold font used in JMenus */
	public static final Font boldFont = new Font("bold_font", Font.PLAIN, 10);
	
	/** the background colour for most items */
	public static final Color BG_COLOUR = new Color(0x594165);
	
	/** The gap between the buttons */
	public static final int ICON_GAP = 3;
	
	/***
	 * Constructs a new <code>AssetMapMenuPanel</code>
	 * 
	 * @param tree the complex asset tree
	 */
	public AssetMapMenuPanel(ComplexAssetTree tree) {
		this.tree = tree;

		menuBar = new JMenuBar();
		menuBar.setBorder(null);
		toolBar = new JToolBar();
		toolBar.setFloatable(false);
		toolBar.setBorderPainted(false);
		toolBar.setBorder(null);
		
		menuBar.setBackground(BG_COLOUR);
	
		setLayout(new BorderLayout());
		
		menuBar.add(createAddMenu());
		toolBar.add(createRefreshAssetsButton());
		toolBar.add(createRestoreRootButton());
		toolBar.add(createCollapseAllButton());
		toolBar.add(createPaintStatusesButton());
	
		JPanel toolPanel = new JPanel();
		toolPanel.setLayout(new FlowLayout(FlowLayout.TRAILING));
		toolPanel.add(toolBar);
		toolPanel.setBackground(BG_COLOUR);
		
		JPanel menuPanel = new JPanel();
		menuPanel.setLayout(new FlowLayout(FlowLayout.LEADING));
		menuPanel.setBackground(BG_COLOUR);
		menuPanel.add(menuBar);

		add(menuPanel, BorderLayout.WEST);
		add(toolPanel, BorderLayout.EAST);

		setBackground(BG_COLOUR);
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
		
		Icon icon = MatrixToolkit.getAssetMapIcon(iconName + "_off.png");
		Icon pressedIcon = MatrixToolkit.getAssetMapIcon(iconName + ".png");
		
		JButton button = new JButton(icon);
		button.setBackground(BG_COLOUR);
		button.setBorderPainted(false);
		
		button.setPreferredSize(new Dimension(icon.getIconWidth() + ICON_GAP, 
			icon.getIconHeight() + ICON_GAP));
		
		button.setPressedIcon(pressedIcon);
		button.addActionListener(listener);
		button.setToolTipText(toolTipText);
		
		return button;
	}
	
	/**
	 * Creates the refresh button.
	 * This button refreshes all assets
	 * that have been expanded during this session.
	 *   
	 * @return the refresh button.
	 */
	private JButton createRefreshAssetsButton() {
		
		ActionListener refreshListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				AssetManager.INSTANCE.reloadAllAssets();
			}
		};
		String toolTip = "Refreshes All assets that have been expanded";
		JButton refreshButton 
			= createButton("refresh", refreshListener, toolTip);

		return refreshButton;
	}

	/**
	* Creates a button to restore the root node 
	* back to the Root Folder (#assetid 1)
	*
	* @return the restore button
	*/
	private JButton createRestoreRootButton() {

		ActionListener restoreListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				tree.restoreRoot();
			}
		};
		
		String toolTip = "Restore the root Folder";
		JButton restoreButton 
			= createButton("teleport", restoreListener, toolTip);
		
		return restoreButton;
	}
	
	/**
	 * Creates the collapse button. Any assets
	 * that are exapanded will be collapsed when this
	 * button is pressed
	 * 
	 * @return the collapse button
	 */
	private JButton createCollapseAllButton() {
	
		ActionListener collapseListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				tree.collapseAllPaths();
				tree.repaint();
			}
		};
		
		String toolTip = "Collapse all";
		JButton collapseButton 
			= createButton("collapse", collapseListener, toolTip);
		
		return collapseButton;
	}
	
	/**
	 * Creates a button to display the status colours.
	 * All assets will have their background colour changed
	 * to display the status colour of the status that the
	 * assets are currently in.
	 * 
	 * @return the paint status button 
	 */
	private JButton createPaintStatusesButton() {
		
		ActionListener statusListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				boolean paintAllStatuses = (tree.paintAllStatuses()) 
					? false 
					: true;
				
				tree.setPaintAllStatuses(paintAllStatuses);
				tree.repaint();
			}
		};

		String toolTip = "Show Status colours";
		JButton paintStatusButton 
			= createButton("status", statusListener, toolTip);
		
		return paintStatusButton;
	}

	/**
	 * Creates the Add menu based on the current user, and the access 
	 * of each <code>AssetType</code>.Icons for each of the 
	 * <code>AssetType</code>s are not loaded until the are explictly 
	 * visible in the menu tree where they exist. 
	 * Ie. when the user rolls over a particular parent menu, the
	 * sub elements's icons are loaded (if they are not already loaded) 
	 * 
	 * @return the addmenu
	 */
	private JMenu createAddMenu() {
		JMenu addMenu = new JMenu();
		Icon icon = MatrixToolkit.getAssetMapIcon("add_off.png");
		addMenu.setIcon(icon);
		addMenu.setPressedIcon(MatrixToolkit.getAssetMapIcon("add.png"));
		addMenu.setFont(new Font("", Font.PLAIN, 10));
		addMenu.setSize(50, 40);
		addMenu.setBackground(BG_COLOUR);
		addMenu.setForeground(new Color(0xFFFFFF));
		
		addMenu.addMenuListener(this);
	
		// get a list of asset types
		Iterator it = AssetManager.INSTANCE.getAssetTypes();

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
			JMenu parentMenu = addMenu;

			if (paths.length > 0) {
				for (int i = 0; i < paths.length; i++) {
					String path = paths[i];
					boolean found = false;
				
					// get the current parent menu, and see if we 
					// already have this path
					for (int j = 0; j < parentMenu.getMenuComponentCount(); j++) {
						Component component = parentMenu.getMenuComponent(j);
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
						newMenu.setFont(boldFont);
						newMenu.getAccessibleContext().setAccessibleName(path);
						newMenu.addMenuListener(this);
						parentMenu.add(newMenu, getMenuIndex(path, parentMenu, newMenu.getClass()));
						
						// if there are no more paths for this menu, 
						// then we want to add this asset type under the
						// newly created menu
			
						if (i ==  paths.length -1) {
							addMenuItem(newMenu, type);
						}
					} else {
						// else we found the JMenu for this path
						// so just add the new asset type under that menu
						addMenuItem(parentMenu, type);

					}//end if not found 
				}//end for
			} else {
				// there are no paths for this asset type so just add it in
				// the root "Add" menu
				addMenuItem(addMenu, type);
		
			}//end if has paths
		}//end while
		
		return addMenu;
	}
	
	/**
	 * Retuns the menu index for the specifed menu element based on its alphabetical
	 * sort value 
	 * 
	 * @param name the name to sort by
	 * @param menu the menu that the element is being added to
	 * 
	 * @return the sort order
	 */
	private int getMenuIndex(String name, JMenu menu, Class cls) {
		Component[] components = menu.getMenuComponents();
		int i = 0;
		for (i = 0; i < components.length; i++) {
			JMenuItem nextMenu = (JMenuItem) menu.getMenuComponent(i);
			
			String otherName = "";
			
			if (nextMenu.getClass().equals(JMenuItem.class)) {
				String typeCode = nextMenu.getAccessibleContext().getAccessibleName();
				otherName = AssetManager.INSTANCE.getAssetType(typeCode).getName();
			} else {
				otherName = nextMenu.getAccessibleContext().getAccessibleName();
			}
			
			// if its just a single item, then we want it down the bottom
			if (cls.equals(JMenuItem.class) && nextMenu.getClass().equals(JMenu.class)) {
				continue;
			}
			
			if (name.compareToIgnoreCase(otherName) < 0) {
				return i;
			}
		}
		return i;
	}
	
	/**
	 * Adds a <code>JMenuItem</code> to a specific <code>JMenu</code>
	 * for a particular <code>AssetType</code>. The newly created 
	 * <code>JMenuItem</code> can be distinguished by its 
	 * <code>AccessibleContext</code>, eg.
	 * 
	 * <pre>
	 * Component[] components = parentMenu.getMenuComponents();
	 * for (int i = 0; i &lt; components.length; i++) {
	 * 	String typeCode 
	 * 		= ((JMenuItem) components[i]).getAccessibleContext().getAccessibleName();
	 * 	AssetType type = AssetTypeFactory.getInstance().getType(typeCode);
	 *      type.loadIcon();
	 * }
	 * </pre>
	 * 
	 * @param parentMenu the menu where the new <code>JMenuItem</code> will 
	 * 			be created
	 * @param type the assetType to reference this <code>JMenuItem</code> to. 
	 *
	 * @return void
	 */
	private void addMenuItem(JMenu parentMenu, AssetType type) {
		JMenuItem newItem = new JMenuItem(type.getName());
		newItem.addActionListener(this);
		newItem.getAccessibleContext().setAccessibleName(type.getTypeCode());
		newItem.setFont(menuFont);
		
		// add single com menu items down the bottom
		parentMenu.add(newItem, getMenuIndex(type.getName(), parentMenu, newItem.getClass()));
	}
	
	/**
	 * Event Listener method that is called when a <code>JMenu</code> item is
	 * expanded from the the Add Menu. Foreach of the <code>JMenuItems</code>,
	 * The type of that item gets asked for its icon, and the type will load it 
	 * if it has not already done so. This will save on the number of initial 
	 * requests.
	 * 
	 * @param e the mouse event
	 * @return void
	 */
	public void menuSelected(MenuEvent e) { 
		JMenu menu = (JMenu) e.getSource();
		Component[] components = menu.getMenuComponents();
		
		for (int i = 0; i < menu.getMenuComponentCount(); i++) {
			if (!(components[i] instanceof JMenu)) {
	
				String typeCode = ((JMenuItem) 
							components[i]).getAccessibleContext().getAccessibleName();
				AssetType type = AssetManager.INSTANCE.getAssetType(typeCode);
				((JMenuItem) components[i]).setIcon(type.getIcon());
			
			}
		}
	}
	
	/**
	 * Called when the menu is cancelled
	 * 
	 * @param e the event
	 */
	public void menuCanceled(MenuEvent e) {}
	
	/**
	 * Called when the menu is deselected
	 * 
	 * @param e the event
	 */
	public void menuDeselected(MenuEvent e) {}

	/**
	 * Called when an action is performed on the Add Menu
	 * 
	 * @param e the event
	 */
	public void actionPerformed(ActionEvent e) {
		JMenuItem item = (JMenuItem) e.getSource();
		String typeCode = item.getAccessibleContext().getAccessibleName();
		AssetType type = null;
		type = AssetManager.INSTANCE.getAssetType(typeCode);
		tree.initNewAsset(type);
	}
}