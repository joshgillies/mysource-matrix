/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: MatrixTreeCellRenderer.java,v 1.9 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

package net.squiz.matrix.matrixtree;

import net.squiz.matrix.core.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.tree.*;
import net.squiz.matrix.ui.*;
import java.awt.event.*;
import net.squiz.matrix.plaf.*;

/**
 * Renders a node for <code>AssetTree</code>.
 *
 * <a name="override">Overrides</a> some painting method for
 * performance reasons.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @see AssetTreeModel
 */
public class MatrixTreeCellRenderer extends JLabel implements TreeCellRenderer, MatrixConstants
{
	/** The asset that this node represents */
	private Asset asset;

	/** The node that is being rendered */
	private MatrixTreeNode node;

	/** TRUE if this node is currently selected */
	private boolean selected = false;

	private boolean allSelected = false;

	public void flipSelection() {
		allSelected = (allSelected) ? false : true;
	}

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

		if (value instanceof MatrixTreeNode) {
			node = (MatrixTreeNode) value;
			Asset asset = node.getAsset();
			this.asset = asset;

			setText(getNodeDisplayText(node));
			setFont(((MatrixTree)tree).getFontInUse());

			if (!isNavNode(node)) {
				setToolTipText(asset.getType().getName()  + " [" + asset.getId() + "]");
				this.selected = selected;
				if (!(asset.isAccessible())) {
					CompoundIcon icon = (CompoundIcon) GUIUtilities.getCompoundIconForTypeCode(
							asset.getType().getTypeCode(), Matrix.getProperty("parameter.url.notaccessibleicon"), asset.getId());
					setIcon(icon);
					setDisabledIcon(icon.getDisabledIcon());
				} else if (node.getLinkType() == LINK_TYPE_2) {
					CompoundIcon icon = (CompoundIcon) GUIUtilities.getCompoundIconForTypeCode(
							asset.getType().getTypeCode(), Matrix.getProperty("parameter.url.type2icon"), asset.getId());
					setIcon(icon);
					setDisabledIcon(icon.getDisabledIcon());
				} else {
					setDisabledIcon(null);
					if (asset.getType() != null)
						setIcon(asset.getType().getIcon());
				}

				if (!MatrixTreeBus.typeIsRestricted(asset.getType())) {
					setEnabled(false);
				} else {
					setEnabled(true);
				}
			} else if (node instanceof ExpandingNode) {
				if (node instanceof ExpandingNextNode) {
					setToolTipText(Matrix.translate("asset_map_tooltip_next_node"));
					setIcon(GUIUtilities.getAssetMapIcon("down_arrows.png"));
				} else {
					setToolTipText(Matrix.translate("asset_map_tooltip_previous_node"));
					setIcon(GUIUtilities.getAssetMapIcon("up_arrows.png"));

				}
				setEnabled(true);
				// If we are not in CueMode and we are using CueMode name then update the name
				if (!((MatrixTree)tree).inCueMode() && ((ExpandingNode)node).usingCueModeName()) {
					((ExpandingNode)node).switchName();
					((DefaultTreeModel) tree.getModel()).nodeChanged(node);
					setText(((ExpandingNode)node).getAssetName());
				}
			} else if (node instanceof LoadingNode) {
				setIcon(GUIUtilities.getAssetMapIcon("loading_node.png"));
			}

		} else if (value instanceof DefaultMutableTreeNode) {
			DefaultMutableTreeNode node = (DefaultMutableTreeNode) value;
			if (node.getUserObject() instanceof String) {
				setText((String) node.getUserObject());
			}
		}



		return this;
	}

	private boolean isNavNode(MatrixTreeNode node) {
		if (!(node instanceof LoadingNode) && !(node instanceof ExpandingNode)) {
			return false;
		}
		return true;
	}

	protected String getNodeDisplayText(MatrixTreeNode node) {
		if ((node instanceof ExpandingNode)) {
			return ((ExpandingNode)node).getAssetName();
		}
		return node.getName() + " ";
	}

	/**
	* Paints the background color that is determined by the selected
	* state of the node, and the status of the asset
	*
	* @param g the graphics
	*/
	public void paint(Graphics g) {
		if (node instanceof ExpandingNode) {
			((ExpandingNode)node).setInitStrWidth(getWidth()+5);
			int width = ((ExpandingNode)node).getInitStrWidth();
			g.setColor(MatrixLookAndFeel.PANEL_COLOUR);
			g.fillRoundRect(0, 0, width, getHeight(), 10, 10);
		}

		if ((selected || allSelected) && !isNavNode(node)) {
			int offset = getIcon().getIconWidth() + (getIconTextGap() / 2);
			if (selected && allSelected) {
				g.setColor(asset.getStatusColour().darker());
			} else {
				g.setColor(asset.getStatusColour());
			}
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
		if (propertyName == "text") {
			super.firePropertyChange(propertyName, oldValue, newValue);
		}
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
