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
* $Id: GUIUtilities.java,v 1.7 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

package net.squiz.matrix.core;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.ui.ErrorDialog;
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

		MatrixTree tree = MatrixTreeBus.getActiveTree();
		if (tree == null) {
			JOptionPane.showMessageDialog(
				comp,
				message,
				title,
				JOptionPane.ERROR_MESSAGE
			);
			return;
		}
		ErrorDialog errorDialog = ErrorDialog.getErrorDialog(message, title, tree.getLocationOnScreen(), tree.getSize());
		errorDialog.show();
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
			String compoundIconName,
			String assetId) {

		String key = "__compound_icon_" + assetId;
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
