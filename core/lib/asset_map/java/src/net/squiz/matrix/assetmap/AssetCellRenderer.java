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
* $Id: AssetCellRenderer.java,v 1.1 2004/01/13 00:39:48 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.awt.Component;
import java.awt.Font;
import java.awt.Color;
import javax.swing.JLabel;
import java.awt.Font;
import javax.swing.JTree;
import javax.swing.Box;

import javax.swing.tree.DefaultTreeCellRenderer;


/**
* Renders a node for <code>AssetTree</code>.
* 
* @author Marc McIntyre <mmcintyre@squiz.net>
* @see AssetTreeModel
*/
public class AssetCellRenderer extends DefaultTreeCellRenderer
{
	/**
	* the asset tree model 
	*/
	private AssetTreeModel model = null;
	

	/** 
	* Constructor.
	* 
	* @param model The tree model that this renderer renders for
	*/
	public AssetCellRenderer(AssetTreeModel model) {
		super();
		this.model = model;

	}//end constructor


	/**
	* Sets the value of the current tree cell to <code>value</code>. 
	* 
	* @param selected		whether the cell will be drawn as if selected
	* @param expanded		whether the node is currently expanded
	* @param leaf			whether the node represents a leaf
	* @param hasFocus		whether the node currently has focus
	* @param row			the row where the node exists in the sub tree
	* @param tree			the <code>JTree</code> the receiver is being configured for.
	* 
	* @return the <code>Component</code> that the renderer uses to draw the value.
	*/
	public Component getTreeCellRendererComponent(
				JTree tree, 		// the tree
				Object value, 		// the value to be painted
				boolean selected, 	// is the node currently selected ?
				boolean expanded, 	// is the node currently expanded ? 
				boolean leaf, 		// if the node a leaf?
				int row, 		// where the node exists in the sub tree
				boolean hasFocus) {	// does the node have focus ?	
		super.getTreeCellRendererComponent(
			tree, value, selected, expanded, leaf, row, hasFocus
		); // super class method returns this anyway

		Asset asset = model.getAssetFromNode(value);
		AssetType assetType = asset.getType();

		setText(asset.getName());
		//set the font for each node
		setFont(new Font("node font", Font.PLAIN, 10));

		setToolTipText(asset.getType().getName()  + " [" + asset.getId() + "]");
		setIcon(assetType.getIcon());

		setEnabled(model.isNodeEnabled(value));

		return this;

	}//end getTreeCellRendererComponent()


}//end class
