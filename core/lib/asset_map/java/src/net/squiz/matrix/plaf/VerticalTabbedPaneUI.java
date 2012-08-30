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
* $Id: VerticalTabbedPaneUI.java,v 1.4 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.plaf;

import java.awt.*;
import javax.swing.plaf.basic.*;

/**
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class VerticalTabbedPaneUI extends BasicTabbedPaneUI {
	private boolean tabsVertical;

	public VerticalTabbedPaneUI(boolean tabsVertical) {
		this.tabsVertical = tabsVertical;
	}

	protected void installDefaults() {
		super.installDefaults();
		if (tabsVertical) {
			// All tab padding
			// Insets(left, top, right, bottom)
			tabAreaInsets = new Insets(2, 2, 0, 0);

			// Individual tab properties
			// Insets(width from left, height top , width from right, height from bottom)
			tabInsets = new Insets(0, 0, 0, 0);

			// Tab content panel insets
			// Insets(top, left, bottom, right)
			contentBorderInsets = new Insets(0, 0, 0, 0);

			// Selected tab insets
			// Insets(height, width, height, width)
			selectedTabPadInsets = new Insets(2, 0, 1, 1);
		}
	}

	protected void paintFocusIndicator(Graphics g,
									int tabPlacement,
									Rectangle[] rects,
									int tabIndex,
									Rectangle iconRect,
									Rectangle textRect,
									boolean isSelected) {

		// Do nothing
	}

	protected void paintTabBorder(Graphics g,
								int tabPlacement,
								int tabIndex,
								int x,
								int y,
								int w,
								int h,
								boolean isSelected) {

		if (!isSelected && tabsVertical) {
			g.setColor(MatrixLookAndFeel.PANEL_BORDER_COLOUR.darker());
			int right_x = x + w - 1;
			g.drawLine(right_x, y, right_x, y+h);
		}

		super.paintTabBorder(g, tabPlacement, tabIndex, x, y, w, h, isSelected);
	}

}
