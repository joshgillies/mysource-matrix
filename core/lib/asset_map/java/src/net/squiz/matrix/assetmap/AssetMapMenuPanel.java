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
* $Id: AssetMapMenuPanel.java,v 1.3 2005/03/06 22:38:47 mmcintyre Exp $
*
*/

package net.squiz.matrix.assetmap;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.inspector.*;
import net.squiz.matrix.debug.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.awt.*;
import java.io.IOException;
import java.util.*;


/**
 * The AssetMapMenuPanel hold various tools to be used with MatrixTrees
 * and InspectorGadgets
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetMapMenuPanel extends JPanel {

	private JMenuBar menuBar;
	private MatrixTree tree;
	private InspectorGadget inspector;
	private JToolBar toolBar;

	public static final Color BG_COLOUR = new Color(0xF5F5F5);
	public static final int ICON_GAP = 1;
	
	/**
	 * Constructs an AssetMapMenuPanel and adds the tools to it.
	 * @return the new AssetMapMenuPanel
	 */
	public AssetMapMenuPanel(MatrixTree tree, InspectorGadget inspector) {
		this.tree = tree;
		this.inspector = inspector;
	
		setLayout(new BorderLayout());
	
		JPanel leftPanel  = new JPanel(new FlowLayout(FlowLayout.LEADING));
		JPanel rightPanel = new JPanel(new FlowLayout(FlowLayout.TRAILING));
		
		rightPanel.add(createRefreshAssetsButton());
		rightPanel.add(createRestoreRootButton());
		rightPanel.add(createCollapseAllButton());
		rightPanel.add(createPaintStatusesButton());
		
		leftPanel.add(createAddMenuButton());
	
		add(leftPanel, BorderLayout.WEST);
		add(rightPanel, BorderLayout.EAST);

		leftPanel.setBackground(BG_COLOUR);
		rightPanel.setBackground(BG_COLOUR);
		setBackground(BG_COLOUR);
	}
	
	public Dimension getPreferredSize() {
		return new Dimension(300, 25);
	}
	
	public Dimension getMinimumSize() {
		return new Dimension(300, 25);
	}
	
	public Dimension getMaximumSize() {
		return new Dimension(300, 25);
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
		
		Icon icon = GUIUtilities.getAssetMapIcon(iconName + "_off.png");
		Icon pressedIcon = GUIUtilities.getAssetMapIcon(iconName + "_on.png");
		
		JButton button = new JButton(icon);
		button.setBackground(BG_COLOUR);
		button.setBorderPainted(false);
		
		button.setPreferredSize(new Dimension(icon.getIconWidth(), icon.getIconHeight()));
		
		button.setPressedIcon(pressedIcon);
		button.addActionListener(listener);
		button.setToolTipText(toolTipText);                 
		
		return button;
	}
	
	/**
	 * Creates the refresh button.
	 * This button refreshes all assets
	 * that have been expanded during this session.
	 * @return the refresh button.
	 */
	private JButton createRefreshAssetsButton() {
		
		ActionListener refreshListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				AssetRefreshWorker worker = new AssetRefreshWorker(true);
				worker.start();
			}
		};
		String toolTip = "Refreshes All Assets";
		JButton refreshButton 
			= createButton("refresh", refreshListener, toolTip);

		return refreshButton;
	}

	/**
	 * Creates a button to restore the root node 
	 * back to the Root Folder (#assetid 1)
	 * @return the restore button
	 */
	private JButton createRestoreRootButton() {

		ActionListener restoreListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				tree.setRootVisible(false);
				((DefaultTreeModel) tree.getModel()).setRoot(AssetManager.getRootFolderNode());
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
	 * @return the collapse button
	 */
	private JButton createCollapseAllButton() {
	
		ActionListener collapseListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				((DefaultTreeModel) tree.getModel()).reload();
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
	 * @return the paint status button 
	 */
	private JButton createPaintStatusesButton() {
		
		ActionListener statusListener = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				JButton button = (JButton) evt.getSource();
				((MatrixTreeCellRenderer) tree.getCellRenderer()).flipSelection();
				((InspectorCellRenderer) inspector.getCellRenderer(0, 0)).flipSelection();
				tree.repaint();
				inspector.repaint();
			}
		};

		String toolTip = "Show Status colours";
		JButton paintStatusButton 
			= createButton("status", statusListener, toolTip);
		
		return paintStatusButton;
	}
	
	/**
	 * Creates the button for the add menu.
	 * @return the button for the add menu
	 */
	private ButtonMenu createAddMenuButton() {
		
		Icon icon = GUIUtilities.getAssetMapIcon("add_off.png");
		Icon pressedIcon = GUIUtilities.getAssetMapIcon("add_on.png");
		
		final ButtonMenu button = new ButtonMenu(icon, pressedIcon);
		
		// we need to do this because the asset map may not have made a request
		// to matrix yet, so the add menu elements might not yet be known
		ActionListener bListener = new ActionListener() {
			private JPopupMenu addMenu;
			
			public void actionPerformed(ActionEvent evt) {
				if (addMenu == null) {
					ActionListener listener = MatrixMenus.getMatrixTreeAddMenuListener(tree);
					addMenu = MatrixMenus.getPopupAddMenu(listener);
					button.setPopupMenu(addMenu);
				}
			}
		};
		
		button.addActionListener(bListener);
		button.setPressedIcon(pressedIcon);
		button.setBackground(BG_COLOUR);
		button.setBorderPainted(false);
		
		button.setPreferredSize(new Dimension(icon.getIconWidth(), icon.getIconHeight()));
		button.setToolTipText("Add New Asset");
		
		return button;
	}
}