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
* $Id: GUIUtilities.java,v 1.2.2.1 2005/06/24 00:45:48 ndvries Exp $
*
*/

package net.squiz.matrix.core;

import javax.swing.*;
import java.awt.*;
import java.net.*;
import java.util.*;
import java.awt.event.*;

public class GUIUtilities {

	/**
	 * A store for Icons. We use this because hitting the server
	 * for an icon everytime it is needed is expensive.
	 */
	private static Map icons = new HashMap();
	private static JPopupMenu addMenu;

	// cannot instantiate
	private GUIUtilities() {}

	/**
	 * Displays an error with the specified message and title using the component
	 * for screen orientation
	 * @param comp the component to use for screen orientation
	 * @param message the message to display
	 * @param title the title to display
	 */
	public static void error(Component comp, String message, String title) {
		JOptionPane.showMessageDialog(
			comp,
			message,
			title,
			JOptionPane.ERROR_MESSAGE
		);
	}

	/**
	 * Displays an error with the specified message and title
	 * the message will be displayed in the center of the screen (l&f dependant)
	 * @param message the message to display
	 * @param title the title to display
	 */
	public static void error(String message, String title) {
		// see the JOptionPane docs about using null as a component
		error(null, message, title);
	}

	/**
	 * Returns an <code>Icon</code> from the generic <code>Icon</code> factory.
	 * Once an <code>Icon</code> has been obtained From the specified source,
	 * it is stored so future resource can obtain the <code>Icon</code>
	 * without hitting the server where the <code>Icon</code> exists.
	 *
	 * @param path the path to the specified <code>Icon</code> source relative to
	 * the Icon directory in the mysource system (/core/lib/web/images/icons).
	 * For example if you wanted an icon for internal messages, this parameter
	 *  should be internal_messages/icon.png
	 * @return the <code>Icon</code> or <code>null</code> if the icon
	 * does not exist at the specified url. If null is returned and is
	 * inserted as the image in a <code>JLabel</code> for example, the image
	 * will be blank and will not affect the execution at all, so consider this
	 * a failsafe device.
	 */
	public static Icon getIcon(String path) {
		if (!icons.containsKey(path)) {
			try {
				Icon icon = new ImageIcon(
						new URL(Matrix.getProperty("parameter.url.baseurl") + path));
				icons.put(path, icon);
				return icon;
			} catch (MalformedURLException mue) {
				return null;
			}
		}
		return (Icon) icons.get(path);
	}

	/**
	 * Returns an icon from the LIB/web/images/icons/asset_map directory
	 * in the mysource matrix system.
	 * @param iconName the name of the icon to get
	 * @return the Icon
	 */
	public static Icon getAssetMapIcon(String iconName) {
		return getIcon(Matrix.getProperty("parameter.url.assetmapiconurl") + "/" + iconName);
	}

	/**
	 * Returns the icon for the given type code
	 * @param typeCode the name of the type code of the wanted icon
	 * @return the Icon
	 */
	public static Icon getIconForTypeCode(String typeCode) {
		return getIcon(Matrix.getProperty("parameter.url.typecodeurl")
			+ "/" + typeCode + "/" + "icon.png");
	}

	/**
	 * Returns whether the icon is loaded for this asset type
	 * @return TRUE if the icon is already loaded
	 */
	public static boolean isIconLoaded(String typeCode) {
		if (icons.containsKey(Matrix.getProperty("parameter.url.typecodeurl") + "/" + typeCode + "/" + "icon.png")) {
			return true;
		} else {
			return false;
		}
	}

	/***
	 * Returns the compound icon for the given type code
	 * @param typeCode the type code of the wanted icon
	 * @param compoundIconName the compound icon name
	 * @return the Icon
	 * @see CompundIcon
	 */
	public static Icon getCompoundIconForTypeCode(
			String typeCode,
			String compoundIconName) {

		String key = "__compound_icon_" + typeCode;
		if (!icons.containsKey(key)) {
			Icon baseIcon = getIconForTypeCode(typeCode);
			Icon overlayIcon = getIcon(Matrix.getProperty("parameter.url.iconurl")
				+ "/" + compoundIconName);

			CompoundIcon icon = new CompoundIcon(
								baseIcon,
								overlayIcon,
								SwingConstants.LEFT,
								SwingConstants.BOTTOM
				);
			icons.put(key, icon);
			return icon;
		}
		return (Icon) icons.get(key);
	}

	/**
	 * Set the location of a Component to the center of the screen
	 * and makes it visible.
	 * @param comp the component to display in the center of the screen
	 */
	public static void showInScreenCenter(Component comp) {
		Dimension size = Toolkit.getDefaultToolkit().getScreenSize();
		int x = (int) (size.getWidth() - comp.getWidth()) / 2;
		int y = (int) (size.getHeight() - comp.getHeight()) / 2;
		comp.setLocation(x, y);
		comp.setVisible(true);
	}

	/**
	 * Returns TRUE if the right mouse button is clicked. Use this in favour of
	 * SwingUtilities.isRightMouseButton() as it detects Mac apple key and mouse click
	 * @param evt the mouse event
	 * @return TRUE if the right mouse button is clicked
	 */
	public static boolean isRightMouseButton(MouseEvent evt) {
		if ((evt.getModifiers() & MouseEvent.BUTTON3_MASK) == MouseEvent.BUTTON3_MASK)
			return true;
		return false;
	}
}
