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
* $Id: VerticalTabbedPane.java,v 1.5 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.ui;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.TreePath;
import javax.swing.plaf.metal.*;

import net.squiz.matrix.assetmap.*;
import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.plaf.*;
import com.sun.java.swing.plaf.windows.*;

public class VerticalTabbedPane extends JTabbedPane {

	private boolean tabsVertical;

	/**
	 * Constructs a JTabbedPane that supports vertica tabs when the
	 * TAB_PLACEMENT is LEFT or RIGHT
	 */
	public VerticalTabbedPane() {
		this(TOP);
	}

	/**
	 * Constructor
	 * @param tabPlacement the placement of the tabs
	 */
	public VerticalTabbedPane(int tabPlacement) {
		super(tabPlacement);
		tabsVertical = (tabPlacement == LEFT || tabPlacement == RIGHT);

		// NdV: Quick hack to solve the color issue in J2SE 1.5
		UIManager.put("TabbedPane.selected", MatrixLookAndFeel.PANEL_COLOUR);

		setUI(new VerticalTabbedPaneUI(tabsVertical));
		// MM: need to do this outside of this class to keep it generic
	/*	addChangeListener(	new ChangeListener() {
			public void stateChanged(ChangeEvent evt) {
				JTabbedPane pane = (JTabbedPane)evt.getSource();
				MatrixTree currentTree = ( (BasicView) ( pane.getComponentAt(pane.getSelectedIndex()) ) ).getTree();

				for (int i = 0; i < pane.getTabCount(); i++) {
					MatrixTree tree = ( (BasicView) ( pane.getComponentAt(i) ) ).getTree();
					TreePath[] path = new TreePath[1];
					path[0] = tree.getCuePath();
					if (path != null) {
						currentTree.startCueMode(path);
						tree.stopCueMode();
						break;
					}
				}
			}
		});*/
	}

	/**
	 * Adds a tab to the tabbedpane
	 * @param s the text on the tab
	 * @param c the component to add to the tab
	 */
	public void addTab(String s, Icon icon, Component c) {
		Icon textIcon = new VerticalTextIcon(s, getTabPlacement() == RIGHT);
		VerticalCompoundIcon compoundIcon = new VerticalCompoundIcon(textIcon, icon, 0, 0);

		insertTab(
			tabsVertical ? null : s,
			tabsVertical ? compoundIcon : null,
			c,
			null,
			getTabCount()
		);

		BufferedImage image = new BufferedImage(
			compoundIcon.getIconWidth(),
			compoundIcon.getIconHeight(),
			BufferedImage.TYPE_INT_ARGB_PRE
		);
		Graphics2D g = image.createGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		compoundIcon.paintIcon(null, g, 0, 0);

		setDisabledIconAt(indexOfComponent(c), new ImageIcon(GrayFilter.createDisabledImage(image)));
	}

    private class VerticalCompoundIcon extends CompoundIcon
    {

        public VerticalCompoundIcon (Icon mainIcon, Icon decorator, int xAlignment, int yAlignment)
        {
			if (!isLegalValue(xAlignment, VALID_X)) {
				throw new IllegalArgumentException(
					"xAlignment must be LEFT, RIGHT or CENTER");
			}
			if (!isLegalValue(yAlignment, VALID_Y)) {
				throw new IllegalArgumentException(
					"yAlignment must be TOP, BOTTOM or CENTER");
			}

			this.mainIcon = mainIcon;
			this.decorator = decorator;
			this.xAlignment = xAlignment;
			this.yAlignment = yAlignment;
        }

		/**
		 * Returns the icon with of the compound icon, which is the same as
		 * the main icon
		 *
		 * @return the width
		 */
		public int getIconWidth() {
			if (mainIcon.getIconWidth() > decorator.getIconWidth()) {
				return mainIcon.getIconWidth() - 7;
			} else {
				return decorator.getIconWidth() - 7;
			}
		}

		/**
		 * Returns the icon height, which is the same as the main icon height
		 *
		 * @return the icon height
		 */
		public int getIconHeight() {
			return mainIcon.getIconHeight() + decorator.getIconHeight();
		}

		/**
		 * Paints the compound icon
		 *
		 * @param c the component
		 * @param g the graphics set
		 * @param x the x co-ordinate
		 * @param y the y co-ordiate
		 */
		public void paintIcon(Component c, Graphics g, int x, int y) {

			int middleX = (x + x + getIconWidth()) / 2;
			int middleY = (y + y + getIconHeight()) / 2;

			Graphics2D g2 = (Graphics2D) g.create();

			int mainIconX = middleX - (mainIcon.getIconWidth() / 2);
			mainIcon.paintIcon(c, g2, mainIconX, y);

			g2.rotate(Math.toRadians(-90), middleX, middleY);

			int decoratorX = middleX - (getIconHeight() / 2);
			int decoratorY = middleY - (decorator.getIconHeight() / 2);

			decorator.paintIcon(
						null,
						g2,
						decoratorX + (mainIcon.getIconWidth() / 2) + 2,
						decoratorY
			);

			g2.dispose();
		}
	}

	private class VerticalTextIcon implements Icon {
		private String tabText;
		private boolean clockwise;

		/**
		 * Constructor
		 * @param tabText the text to paint for the icon
		 * @param clockwise if TRUE the text will be rotated clockwise
		 */
		public VerticalTextIcon(String tabText, boolean clockwise) {
			this.tabText = tabText;
			this.clockwise = clockwise;
		}

		/**
		 * Paints the icon. draws the text vertical if the TAB_PLACEMENT is
		 * LEFT of RIGHT.
		 * @param c the component
		 * @param g the graphics to paint into
		 * @param x the x co-ordinate
		 * @param y the y-co-ordiate
		 */
		public void paintIcon(Component c, Graphics g, int x, int y) {
			Graphics2D g2 = (Graphics2D) g;

			g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

			// rotate the graphics 90 degress and draw the string
			// clockwise for the right, anti-clockwise for the left
			g2.rotate(Math.toRadians(clockwise ? 90 : -90), x, y);
			g2.setFont((Font)UIManager.get("VerticalTextIcon.font"));

			g2.drawString(
				tabText,
				x - (clockwise ? 0 : getIconHeight()) + 8,
				y + (clockwise ? 0 : getIconWidth()));
				g2.rotate (Math.toRadians(clockwise ? - 90 : 90), x, y
			);
		}

		/**
		 * Returns the icon width
		 * @return the icon width
		 */
		public int getIconWidth() {
			return 8;
		}

		/**
		 * Returns the icon height
		 * @return the icon height
		 */
		public int getIconHeight() {
			return getFontMetrics(getFont()).stringWidth(tabText) + 20;
		}
	}
}
