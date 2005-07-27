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
* $Id: InspectorCellRenderer.java,v 1.2 2005/07/27 10:45:22 brobertson Exp $
*
*/

/**
 * :tabSize=4:indentSize=4:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 */

package net.squiz.matrix.inspector;

import net.squiz.matrix.core.*;
import net.squiz.matrix.matrixtree.*;
import javax.swing.*;
import javax.swing.JTable;
import javax.swing.JLabel;
import javax.swing.SwingConstants;
import javax.swing.table.TableCellRenderer;
import javax.swing.tree.*;
import javax.swing.border.LineBorder;

import java.awt.*;
import java.awt.Component;

/**
 * Renders a node for <code>InspectorGadget</code>.
 *
 * <a name="override">Overrides</a> some painting method for
 * performance reasons.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 * @see InspectorGadget
 */
public class InspectorCellRenderer 	extends 	JLabel
									implements 	TableCellRenderer,
												MatrixConstants {

	/** The asset that this node represents */
	private Asset asset;

	private boolean selected = false;

	private boolean allSelected = false;


	//{{{ Public Methods

	/**
	 * Sets the value of the current table cell to <code>value</code>.
	 *
	 * @param table - the <code>JTable</code>
	 * @param value - the value to assign to the cell at [row, column]
	 * @param isSelected - true if cell is selected
	 * @param hasFocus - true if cell has focus
	 * @param row - the row of the cell to render
	 * @param column - the column of the cell to render
	 * @return the <code>Component</code> that the renderer uses to draw the value.
	 */
	public Component getTableCellRendererComponent(
		JTable table,
		Object value,
		boolean isSelected,
		boolean hasFocus,
		int row,
		int column) {

			if (value == null)
				return null;

			else if (value instanceof MatrixTreeNode) {
				MatrixTreeNode node = (MatrixTreeNode) value;
				Asset asset = node.getAsset();
				this.asset = asset;
				this.selected = isSelected;

				String name = asset.getName();
				int len = 12;
				if (name.length() > len) {
					name = name.substring(0, len - 3) + "...";
				}
				String numAssetsStr = null;
				int numKids = node.getAsset().getNumKids();
				if (numKids < 0) {
					numAssetsStr = "unknown";
				} else if (numKids == 1) {
					numAssetsStr = numKids + " asset";
				} else {
					numAssetsStr = numKids + " assets";
				}

				setText("<html><center>" + name + "<br><font color=\"#AAAAAA\">" + numAssetsStr + "</font></center></html>");
				String toolTip = new String("<html>" +
											asset.getName() + "<br>" +
											asset.getType().getName() + " [" + asset.getId() + "]" +
											"</html>");
				setToolTipText(toolTip);
				setFont(PLAIN_FONT_10);
				setDisabledIcon(null);

				if (asset.getType() != null) {
					setIcon(asset.getType().getIcon());
				}
				setVerticalTextPosition(JLabel.BOTTOM);
				setHorizontalTextPosition(JLabel.CENTER);
				setHorizontalAlignment(JLabel.CENTER);
				setIconTextGap(1);
			} else if (value instanceof DefaultMutableTreeNode) {
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
		Graphics2D g2;
		int iconSquareOffset = (int) ( getSize().getWidth() - getIcon().getIconWidth() ) / 2;
		int iconSquareWidth = getIcon().getIconWidth() + 10;
		int iconSquareHeight = getIcon().getIconHeight() + 10;

		if (selected || allSelected) {
			FontMetrics fm = getFontMetrics(getFont());

			int textWidth = fm.stringWidth(getText());
			textWidth = textWidth > getWidth() ? getWidth() : textWidth;
			textWidth = textWidth - 1 < iconSquareWidth ? iconSquareWidth + 1 : textWidth;
			textWidth = textWidth % 2 != 0 ? textWidth++ : textWidth;

			int offset = ((getWidth() - textWidth) / 2) + 1;

			int textOffset = getIcon().getIconHeight() + getIconTextGap() + 3;
			int height = fm.getHeight() - 2;

			//g.setColor(new Color(0,0,128));

			// Highlight text only
			/*g.setColor(asset.getStatusColour());
			g.fillRect(offset, textOffset, textWidth - 1, height);
			g.setColor(asset.getStatusColour().darker());
			g.drawRect(offset, textOffset, textWidth - 1, height);*/

			// Highlight text only, but the width of the cell rather than the text
			g.setColor(asset.getStatusColour());
			g.fillRect(0, textOffset, getWidth() - 1, height);
			g.setColor(asset.getStatusColour().darker());
			g.drawRect(0, textOffset, getWidth() - 1, height);

			// Highlight entire cell
			/*g.setColor(asset.getStatusColour());
			g.fillRect(0, 0, getWidth() - 1, getHeight());
			g.setColor(asset.getStatusColour().darker());
			g.drawRect(0, 0, getWidth() - 1, getHeight());*/

			//g.setColor(asset.getStatusColour().darker());


			//g2 = (Graphics2D)g;
			//g2.setStroke(new BasicStroke(2));
			//g2.drawRect(0,0, getWidth() - 2, getWidth() - 2);
			//g.drawRect(iconSquareOffset - 5, 1, iconSquareWidth, iconSquareHeight);
			//g = (Graphics)g2;
			//setForeground(Color.WHITE);
		}
		else {
			//g2 = (Graphics2D)g;
			//g2.setStroke(new BasicStroke(1));
			//g2.setColor(new Color(192,192,192));
			//g.drawRect(iconSquareOffset - 5, 1, iconSquareWidth, iconSquareHeight);
			//g2.drawRect(0,0, getWidth() - 2, iconSquareHeight);

			//g = (Graphics)g2;
			//setForeground(Color.BLACK);
		}

		super.paint(g);
	}

	/**
	 * Flips the selection so that the user can see the status color of all the
	 * assets
	 *
	 */
	public void flipSelection() {
		allSelected = (allSelected) ? false : true;
	}

	//}}}

	//{{{ Protected Methods
	//}}}

	//{{{ Package Private Methods
	//}}}

	//{{{ Private Methods
	//}}}

	//{{{ Protected Inner Classes
	//}}}

	//{{{ Inner Classes
	//}}}
}
