

package net.squiz.matrix.complexassetmap;

import javax.swing.*;
import java.awt.*;
import java.awt.image.*;
import javax.swing.border.*;
import net.squiz.matrix.assetmap.*;

class StatusKey extends JPanel {

	public static final int KEY_SIZE = 15;
	public static final Color KEY_BORDER = new Color(0xCCCCCC);
	public static final Font keyFont = new Font("key_font", Font.PLAIN, 10);
	public static final Dimension fillerSize = new Dimension(1, 1);
	
	public StatusKey() {
		createKey();
		setBackground(Color.white);
		setBorder(new EmptyBorder(3, 3, 3, 3));
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	}
	
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
		g.setColor(KEY_BORDER);
		g.drawRect(0, 0, KEY_SIZE - 1, KEY_SIZE - 1);
		g.dispose();

		ImageIcon icon = new ImageIcon(image);
		
		return icon;
	}
}
