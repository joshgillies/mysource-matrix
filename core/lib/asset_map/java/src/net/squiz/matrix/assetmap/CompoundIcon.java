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
* $Id: CompoundIcon.java,v 1.5 2006/12/05 05:26:35 bcaldwell Exp $
*
*/

package net.squiz.matrix.assetmap;


import java.awt.*;
import javax.swing.*;

/**
 * The compond icon can be used to take an existing icon and overlay
 * another smaller icon over it.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class CompoundIcon implements Icon, SwingConstants {

	/** The valid x positions */
	protected static final int[]
    VALID_X = {LEFT, RIGHT, CENTER};

	/** The valid y positions */
	protected static final int[]
    VALID_Y = {TOP, BOTTOM, CENTER};

	/** The main and decorator icons */
	protected Icon mainIcon, decorator;

	/** The y alignment */
	protected int yAlignment = BOTTOM;

	/** The x alignment */
	protected int xAlignment = LEFT;

	/**
	 * Constructor with empty args so that the class can be extended
	 *
	 */
	public CompoundIcon() {

	}

	/**
	 * Constructs a conpound icon
	 *
	 * @param mainIcon the main icon which will be on the bottom layer
	 * @param decorator the decorator icon which will be on the top layer
	 * @param xAlignment the x alignment
	 * @param yAlignment the y alignment
	 */
	public CompoundIcon(
	  		Icon mainIcon,
			Icon decorator,
			int xAlignment,
			int yAlignment) {
		if (decorator.getIconWidth() > mainIcon.getIconWidth()) {
	  		throw new IllegalArgumentException(
	  			"decorator icon is wider than main icon");
	    }
		if (decorator.getIconHeight() > mainIcon.getIconHeight()) {
	    	throw new IllegalArgumentException(
	        	"decorator icon is higher than main icon");
	    }
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
	 * Returns TRUE if the specified value is legal
	 *
	 * @param value the value to check
	 * @param legal the legal values
	 * @return
	 */
	public boolean isLegalValue(int value, int[] legal) {
		for (int i = 0; i < legal.length; i++) {
			if (value == legal[i])
				return true;
		}
		return false;
	}

	/**
	 * Returns an icon that can be used in disabled JLabels
	 *
	 * @return the disabled icon
	 */
	public Icon getDisabledIcon() {
		Image grayImage = GrayFilter.createDisabledImage(((ImageIcon) mainIcon).getImage());
        return new ImageIcon(grayImage);
	}

	/**
	 * Returns the icon with of the compond icon, wchich is the sam with
	 * as the main icon
	 *
	 * @return the width
	 */
	public int getIconWidth() {
		return mainIcon.getIconWidth();
	}

	/**
	 * Returns the icon height, which is the same as the main icon height
	 *
	 * @return the icon height
	 */
	public int getIconHeight() {
		return mainIcon.getIconHeight();
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
		mainIcon.paintIcon(c, g, x, y);
		int w = getIconWidth();
		int h = getIconHeight();

		if (xAlignment == CENTER) {
			x += (w - decorator.getIconWidth()) / 2;
		}
		if (xAlignment == RIGHT) {
			x += (w - decorator.getIconWidth());
		}
		if (yAlignment == CENTER) {
			y += (h - decorator.getIconHeight()) / 2;
		}
		if (yAlignment == BOTTOM) {
			y += (h - decorator.getIconHeight());
		}
		decorator.paintIcon(c, g, x, y);
	}
}

