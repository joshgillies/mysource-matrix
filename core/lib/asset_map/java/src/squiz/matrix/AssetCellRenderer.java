package squiz.matrix;

import java.awt.Component;

import java.awt.Font;
import java.awt.Color;

import javax.swing.JLabel;
import javax.swing.JTree;
import javax.swing.Box;

import javax.swing.tree.DefaultTreeCellRenderer;
/**
 * Renders a node for <code>AssetTree</code>.
 * 
 * <p><code>$Id: AssetCellRenderer.java,v 1.1 2003/11/14 05:21:36 dwong Exp $</code></p>
 *
 * @author		Dominic Wong <dwong@squiz.net>
 * @version		$Version$
 * @see			AssetTreeModel
 */

public class AssetCellRenderer extends DefaultTreeCellRenderer
{
	/** the asset tree model */
	private AssetTreeModel model;
	/** The AssetTree - so we have something to call back for popup menu */

	/** 
	 * Constructor.
	 * 
	 * @param model		The tree model that this renderer renders for
	 */
	public AssetCellRenderer(AssetTreeModel model) {
		super();
		this.model = model;
	}

	/**
	 * Sets the value of the current tree cell to <code>value</code>. 
	 * 
	 * @param selected		whether the cell will be drawn as if selected
	 * @param expanded		whether the node is currently expanded
	 * @param leaf			whether the node represents a leaf
	 * @param hasFocus		whether the node currently has focus
	 * @param tree			the <code>JTree</code> the receiver is being 
	 *						configured for. 
	 * @return				the <code>Component</code> that the renderer uses to draw the value.
	 */
	public Component getTreeCellRendererComponent(
				JTree tree, 
				Object value, 
				boolean selected, 
				boolean expanded, 
				boolean leaf, 
				int row, 
				boolean hasFocus) {
		super.getTreeCellRendererComponent(
			tree, value, selected, expanded, leaf, row, hasFocus
		); // super class method returns this anyway

		Asset asset = model.getAssetFromNode(value);
		AssetType assetType = asset.getType();

		setText(asset.getName());
		setToolTipText(asset.getType().getName()  + " [" + asset.id() + "]");
		setIcon(assetType.getIcon());

		setEnabled(model.isNodeEnabled(value));

		return this;
	}//end getTreeCellRendererComponent()

}//end class