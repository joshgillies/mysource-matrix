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
* $Id: StatusKey.java,v 1.5 2006/12/05 05:26:37 bcaldwell Exp $
*
*/


package net.squiz.matrix.ui;

import javax.swing.*;
import java.awt.*;
import java.awt.image.*;
import javax.swing.border.*;
import net.squiz.matrix.core.*;

/**
* Creates a panel with a key of the status colours used in the asset map
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*/
public class StatusKey extends JPanel implements MatrixConstants {

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
		setMinimumSize(new Dimension(0,0));
		setMaximumSize(new Dimension(0,200));
		setPreferredSize(new Dimension(0,200));
	}

	/**
	* Creates the key and adds the conponents to the panel
	*/
	private void createKey() {
		add(createColourKey(ARCHIVED_COLOUR, "Archived"));
		add(createColourKey(UNDER_CONSTRUCTION_COLOUR, "Under Construction"));
		add(createColourKey(PENDING_APPROVAL_COLOUR, "Pending Approval"));
		add(createColourKey(APPROVED_COLOUR, "Approved"));
		add(createColourKey(LIVE_COLOUR, "Live"));
		add(createColourKey(LIVE_APPROVAL_COLOUR, "Up For Review"));
		add(createColourKey(EDITING_COLOUR, "Safe Editing"));
		add(createColourKey(EDITING_APPROVAL_COLOUR, "Safe Edit Pending Approval"));
		add(createColourKey(EDITING_APPROVED_COLOUR, "Safe Edit Approved"));
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
