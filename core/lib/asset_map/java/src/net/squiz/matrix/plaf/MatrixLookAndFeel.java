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
* $Id: MatrixLookAndFeel.java,v 1.6 2012/08/30 01:09:21 ewang Exp $
*
*/

package net.squiz.matrix.plaf;

import net.squiz.matrix.core.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.plaf.metal.*;
import com.sun.java.swing.plaf.windows.*;
import java.awt.*;
import javax.swing.plaf.*;

public class MatrixLookAndFeel extends MetalLookAndFeel implements MatrixConstants {

	public static Color PANEL_COLOUR = new Color(0xF5F5F5);
	public static Color PANEL_BORDER_COLOUR = new Color(0xC3C3C3);

	protected void initClassDefaults(UIDefaults table) {
		super.initClassDefaults(table);
		String packageName = "net.squiz.matrix.plaf.";
		Object[] uiDefaults = {
			"SplitPaneUI", packageName + "MatrixSplitPaneUI",
		};

		table.putDefaults(uiDefaults);
	}

	protected void initComponentDefaults(UIDefaults table) {
		super.initComponentDefaults(table);

		Object[] defaults = {
			"StatusBar.border", new LineBorder(PANEL_BORDER_COLOUR),
			"StatusBar.background", PANEL_COLOUR,
			"SplitPane.border", new LineBorder(PANEL_BORDER_COLOUR),
			"SplitPaneDivider.buttonbackground", PANEL_COLOUR, // matrix specific
			"SplitPaneDivider.buttonforeground", PANEL_BORDER_COLOUR, // matrix specific
			"SplitPaneDivider.buttonbordercolor", new Color(0x3B3B3B), // matrix specific
			"SplitPaneDivider.border", new LineBorder(PANEL_BORDER_COLOUR),

			"SplitPane.background", PANEL_COLOUR,
			"Tree.expandedIcon", new WindowsTreeUI.ExpandedIcon(),
			"Tree.collapsedIcon", new WindowsTreeUI.CollapsedIcon(),
			//"ScrollBar.shadow", new Color(0x000000), // left line in scrollbar track
			"ScrollBar.thumb", PANEL_COLOUR,  // background of the scrollbar
			"ScrollBar.thumbShadow", PANEL_BORDER_COLOUR, // scrollbar border
			"ScrollBar.thumbHighlight", PANEL_COLOUR, // scrollbar dots
			"ScrollBar.track", PANEL_COLOUR,
			"ScrollBar.trackHighlightColor", PANEL_COLOUR,
			"ScrollBar.background", PANEL_COLOUR, // track background
			//"ScrollBar.foreground", Color.blue,
			"ScrollBar.darkShadow", PANEL_BORDER_COLOUR,

			"TabbedPane.tabAreaBackground", Color.RED, // doesnt seem to do much
			"TabbedPane.selected", PANEL_COLOUR, // selected tab

			"TabbedPane.selectHighlight", PANEL_BORDER_COLOUR, // selected tab border
			"TabbedPane.background", PANEL_BORDER_COLOUR, // other tabs

			"TabbedPane.foreground", Color.BLACK, // text color
			"TabbedPane.light", PANEL_BORDER_COLOUR.brighter(), // other tab border
			"TabbedPane.highlight", PANEL_BORDER_COLOUR.brighter().brighter(), // nothing
			"TabbedPane.shadow", PANEL_BORDER_COLOUR.darker(), // all tab borders
			"TabbedPane.darkShadow", PANEL_BORDER_COLOUR.darker().darker(), // all tab borders
			"TabbedPane.font", PLAIN_FONT_10,

			"VerticalTextIcon.font", PLAIN_FONT_10,

			//"SplitPane.dividerSize", new Integer(10),

			"MenuItem.background", PANEL_COLOUR,
			"Menu.background", PANEL_COLOUR,
			//"MenuItem.border", new LineBorder(PANEL_BORDER_COLOUR),
			//"Menu.border", PANEL_BORDER_COLOUR,
			"PopupMenu.border", new LineBorder(PANEL_BORDER_COLOUR),
			"MenuItem.selectionBackground", PANEL_BORDER_COLOUR,
			"Menu.selectionBackground", PANEL_BORDER_COLOUR,

			"SelectionTool.background", PANEL_BORDER_COLOUR, // matrix specific
			"SelectionTool.bordercolor", PANEL_BORDER_COLOUR, // matrix specific

			"InspectorNavigator.background", PANEL_COLOUR,

			"MenuItem.font", PLAIN_FONT_10,
			"Menu.font", PLAIN_FONT_10,

			"CueLine.stroke", Color.BLACK,

		};
		table.putDefaults(defaults);
	}

	protected void initSystemColorDefaults(UIDefaults table) {
		super.initSystemColorDefaults(table);

		Object[] systemColors = {
			"scrollbar", PANEL_BORDER_COLOUR,
		};

		for (int i = 0; i < systemColors.length; i += 2) {
			table.put((String) systemColors[i], systemColors[i + 1]);
		}
	}
}
