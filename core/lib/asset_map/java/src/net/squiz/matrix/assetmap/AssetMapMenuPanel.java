package net.squiz.matrix.assetmap;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.inspector.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.ui.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.awt.*;
import java.util.*;


public class AssetMapMenuPanel extends JPanel {

	private JMenuBar menuBar;
	private MatrixTree tree;
	private InspectorGadget inspector;
	private JToolBar toolBar;

	public static final Color BG_COLOUR = new Color(0xF5F5F5);
	public static final int ICON_GAP = 1;
	
	public AssetMapMenuPanel(MatrixTree tree, InspectorGadget inspector) {
		this.tree = tree;
		this.inspector = inspector;
	//	toolBar = new JToolBar();
	//	toolBar.setFloatable(false);
	//	toolBar.setBorderPainted(false);
	//	toolBar.setBorder(null);
	
		setLayout(new FlowLayout(FlowLayout.TRAILING));
		
		add(createRefreshAssetsButton());
		add(createRestoreRootButton());
		add(createCollapseAllButton());
		add(createPaintStatusesButton());
	
	//	JPanel toolPanel = new JPanel();
	//	toolPanel.setLayout(new FlowLayout(FlowLayout.TRAILING));
	//	toolPanel.add(toolBar);
	//	toolPanel.setBackground(BG_COLOUR);

	//	add(toolPanel);

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
		
		button.setPreferredSize(new Dimension(icon.getIconWidth()/* + ICON_GAP*/, 
			icon.getIconHeight()));
		
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
				MatrixStatusBar.setStatus("Refreshing All Assets");
				SwingWorker worker = new SwingWorker() {
					public Object construct() {
						AssetManager.refreshAllKnownAssets();
						MatrixStatusBar.setStatusAndClear("Success!", 1000);
						return null;
					}
				};
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
	*
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
	 * 
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
	 * 
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
}