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
* $Id: AssetTreeCellRenderer.java,v 1.1 2004/06/29 01:23:36 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.awt.*;
import javax.swing.*;
import javax.swing.tree.*;

/**
 * Renders a node for <code>AssetTree</code>.
 * 
 * <a name="override">Overrides</a> some painting method for 
 * performance reasons. 
 * 
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @see AssetTreeModel
 */
public class AssetTreeCellRenderer extends JLabel implements TreeCellRenderer
{
	/** The asset that this node represents */
	private Asset asset;
	
	/** TRUE if this node is currently selected */
	private boolean selected = false;
	
	/**
	 * Sets the value of the current tree cell to <code>value</code>. 
	 * 
	 * @param selected		whether the cell will be drawn as if selected
	 * @param expanded		whether the node is currently expanded
	 * @param leaf			whether the node represents a leaf
	 * @param hasFocus		whether the node currently has focus
	 * @param row			the row where the node exists in the sub tree
	 * @param tree			the <code>JTree</code> the receiver is being configured for.
	 * @return the <code>Component</code> that the renderer uses to draw the value.
	 */
	public Component getTreeCellRendererComponent(
				JTree tree,
				Object value,
				boolean selected,
				boolean expanded,
				boolean leaf,
				int row,
				boolean hasFocus) {
		
		if (value instanceof AssetTreeNode) {
			
			AssetTreeNode node = (AssetTreeNode) value;
			Asset asset = node.getAsset();
			this.asset = asset;
			AssetTree assetTree = (AssetTree) tree;
			
			if (assetTree.paintAllStatuses()) {
				this.selected = true;
			} else {
				this.selected = selected;
			}

			setText(asset.getName() + " ");
			setFont(AssetTreeNode.NODE_FONT);
			setForeground(AssetTreeNode.NODE_FONT_COLOR);
			setToolTipText(asset.getType().getName()  + " [" + asset.getId() + "]");
			
			if (!(asset.isAccessible())) {
				setIcon(MatrixToolkit.getCompoundIconForTypeCode(
						asset.getType().getTypeCode(), MatrixToolkit.NOT_ACCESSIBLE_ICON));
			} else if (asset.isType2Link()) {
				setIcon(MatrixToolkit.getCompoundIconForTypeCode(
						asset.getType().getTypeCode(), MatrixToolkit.TYPE_2_ICON));
			} else {
				setIcon(asset.getType().getIcon());
			}
			
			if ((assetTree.assetFinderMode())
					&& (!assetTree.noTypesRestricted() 
					&& !assetTree.typeIsRestricted(asset.getType()))) {
				setEnabled(false);
			} else {
				setEnabled(true);
			}
			
		} else if (value instanceof DefaultMutableTreeNode) {
			// we will assume that this is the loading node
			DefaultMutableTreeNode node = (DefaultMutableTreeNode) value; 
			if (node.getUserObject() instanceof String) {	
				setText((String) node.getUserObject());
			}
		}
		return this;
	}
	
	/**
     * Paints the background color that is determined by the selected
     * state of the node, and the status of the asset
     *
     * @param g the graphics
     */
    public void paint(Graphics g) {
                                                                                                                                                                                
        if (selected) {
            int offset = getIcon().getIconWidth() + (getIconTextGap() / 2);
            g.setColor(asset.getStatusColour());
            g.fillRect(offset, 2, getWidth(), getHeight() - 4);
            g.setColor(asset.getStatusColour().darker());
            g.drawRect(offset, 2,  getWidth() - offset - 1, getHeight() - 4);
                                                                                                                                                                            
        }
        super.paint(g);
    }

	/**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void validate() {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void revalidate() {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void repaint(long tm, int x, int y, int width, int height) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void repaint(Rectangle r) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    protected void firePropertyChange(String propertyName, Object oldValue, Object newValue) {	
    	if (propertyName == "text")
    		super.firePropertyChange(propertyName, oldValue, newValue);
    }

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void firePropertyChange(String propertyName, byte oldValue, byte newValue) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void firePropertyChange(String propertyName, char oldValue, char newValue) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void firePropertyChange(String propertyName, short oldValue, short newValue) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void firePropertyChange(String propertyName, int oldValue, int newValue) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void firePropertyChange(String propertyName, long oldValue, long newValue) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void firePropertyChange(String propertyName, float oldValue, float newValue) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
    public void firePropertyChange(String propertyName, double oldValue, double newValue) {}

   /**
    * Overridden for performance reasons.
    * See the <a href="#override">Implementation Note</a>
    * for more information.
    */
	public void firePropertyChange(String propertyName, boolean oldValue, boolean newValue) {}
}
