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
* $Id: StatusKey.java,v 1.2 2004/09/26 23:15:12 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


package net.squiz.matrix.complexassetmap;

import javax.swing.*;
import java.awt.*;
import java.awt.image.*;
import javax.swing.border.*;
import net.squiz.matrix.assetmap.*;

/**
* Creates a panel with a key of the status colours used in the asset map
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*/
class StatusKey extends JPanel {

	public static final int KEY_SIZE = 15;
	public static final Color KEY_BORDER = new Color(0xCCCCCC);
	public static final Font keyFont = new Font("key_font", Font.PLAIN, 10);
	public static final Dimension fillerSize = new Dimension(1, 1);
	
	/**
	* Constructs a status key
	*/
	public StatusKey() {
		createKey();
		setBackground(Color.white);
		setBorder(new EmptyBorder(3, 3, 3, 3));
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	}
	
	/**
	* Creates the key and adds the conponents to the panel
	*/
	private void createKey() {
		add(createColourKey(Asset.ARCHIVED_COLOUR, "Archived"));
		add(createColourKey(Asset.UNDER_CONSTRUCTION_COLOUR, "Under Construction"));
		add(createColourKey(Asset.PENDING_APPROVAL_COLOUR, "Pending Approval"));
		add(createColourKey(Asset.APPROVED_COLOUR, "Approved"));
		add(createColourKey(Asset.LIVE_COLOUR, "Live"));
		add(createColourKey(Asset.LIVE_APPROVAL_COLOUR, "Up For Review"));
		add(createColourKey(Asset.EDITING_COLOUR, "Safe Editing"));
		add(createColourKey(Asset.EDITING_APPROVAL_COLOUR, "Safe Edit Pending Approval"));
		add(createColourKey(Asset.EDITING_APPROVED_COLOUR, "Safe Edit Approved"));
	}
	
	private JLabel createColourKey(Color c, String labelText) {
		JLabel label = new JLabel(labelText);
		label.setIcon(createIcon(c));
		label.setFont(keyFont);

		add(new Box.Filler(fillerSize, fillerSize, fillerSize));
		
		return label;
	}
	
	private Icon createIcon(Color c) {
		
		BufferedImage image = new BufferedImage(KEY_SIZE, KEY_SIZE, BufferedImage.TYPE_INT_RGB);
		Graphics g = image.createGraphics();

		g.setColor(c);
		g.fillRect(0, 0, KEY_SIZE, KEY_SIZE);
		g.setColor(c.darker());
		g.drawRect(0, 0, KEY_SIZE - 1, KEY_SIZE - 1);
		g.dispose();

		ImageIcon icon = new ImageIcon(image);
		
		return icon;
	}
}
