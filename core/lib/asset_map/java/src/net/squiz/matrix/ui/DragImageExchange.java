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
* $Id: DragImageExchange.java,v 1.2 2006/12/05 05:26:37 bcaldwell Exp $
*
*/

package net.squiz.matrix.ui;

import java.awt.image.BufferedImage;
import java.awt.Point;

/**
 * DragImageExchange provides a means for moving sharing ghosted drag images
 * between different components. When a drag operation starts, the source
 * component renders the drag image and stores it in this class. The target
 * component can then use the image when painting the ghosted drop image.
 *
 * @author Nathan de Vries <ndvries@squiz.net>
 */
public class DragImageExchange {

	private static BufferedImage dragImage;
	private static Point mouseOffset;
	private static boolean inExchange = false;

	// cannot instantiate
	private DragImageExchange() {}

	/**
	 * Stores an drag image for later use.
	 *
	 * @param dragImage  a drag image which represents the dragged items
	 * @param mouseOffset  the offset between the mouse and the top-left corner
	 *                     of the dragImage
	 */
	public static void setDragImage(BufferedImage dragImage, Point mouseOffset) {
		//if (inExchange)
		//	throw new IllegalStateException("There is already an exchange open.");
		DragImageExchange.dragImage = dragImage;
		DragImageExchange.mouseOffset = mouseOffset;
		inExchange = true;
	}

	/**
	 * Returns the stored drag image
	 */
	public static BufferedImage getDragImage() {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open.");
		return dragImage;
	}

	/**
	 * Returns the offset between the stored drag image and the original mouse
	 * pointer.
	 */
	public static Point getMouseOffset() {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open.");
		return mouseOffset;
	}

	/**
	 * Ends the current image exchange.
	 */
	public static void completeExchange() {
		if (!inExchange)
			throw new IllegalStateException("There is no exchange open.");
		dragImage = null;
		inExchange = false;
	}
}
