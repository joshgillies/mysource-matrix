
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
	public void addTab(String s, Component c) {
		Icon icon = new VerticalTextIcon(s, getTabPlacement() == RIGHT);
		insertTab(
			tabsVertical ? null : s,
			tabsVertical ? icon : null,
			c,
			null,
			getTabCount()
		);
		BufferedImage image = new BufferedImage(
			icon.getIconWidth(),
			icon.getIconHeight(),
			BufferedImage.TYPE_INT_ARGB_PRE
		);
		Graphics2D g = image.createGraphics();
		icon.paintIcon(null, g, 0, 0);

		setDisabledIconAt(indexOfComponent(c), new ImageIcon(GrayFilter.createDisabledImage(image)));
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
			// g2.setFont(UIManger.getVertical);
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
			return 10;
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
