
package net.squiz.matrix.plaf;

import net.squiz.matrix.core.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.plaf.metal.*;
import com.sun.java.swing.plaf.windows.*;
import java.awt.*;
import javax.swing.plaf.*;

public class MatrixLookAndFeel extends MetalLookAndFeel implements MatrixConstants {

	private static Color lightGrey = new Color(0xC3C3C3);
	private static Color vLightGrey = new Color(0xF5F5F5);

	protected void initClassDefaults(UIDefaults table) {
		super.initClassDefaults(table);
		String packageName = "net.squiz.matrix.plaf.";
		System.out.println("installing matrix specified UI classes");
		Object[] uiDefaults = {
			"SplitPaneUI", packageName + "MatrixSplitPaneUI",
		};

		table.putDefaults(uiDefaults);
	}

	protected void initComponentDefaults(UIDefaults table) {
		super.initComponentDefaults(table);

		Object[] defaults = {
			"StatusBar.border", new LineBorder(lightGrey),
			"StatusBar.background", vLightGrey,
			"SplitPane.border", new LineBorder(lightGrey),
			"SplitPaneDivider.buttonbackground", vLightGrey, // matrix specific
			"SplitPaneDivider.buttonforeground", lightGrey, // matrix specific
			"SplitPaneDivider.buttonbordercolor", new Color(0x3B3B3B), // matrix specific
			"SplitPaneDivider.border", new LineBorder(lightGrey),

			"SplitPane.background", vLightGrey,
			"Tree.expandedIcon", new WindowsTreeUI.ExpandedIcon(),
			"Tree.collapsedIcon", new WindowsTreeUI.CollapsedIcon(),
		//	"ScrollBar.shadow", new Color(0x000000), // left line in scrollbar track
			"ScrollBar.thumb", vLightGrey,  // background of the scrollbar
			"ScrollBar.thumbShadow", lightGrey, // scrollbar border
			"ScrollBar.thumbHighlight", vLightGrey, // scrollbar dots
			"ScrollBar.track", vLightGrey,
			"ScrollBar.trackHighlightColor", vLightGrey,
			"ScrollBar.background", vLightGrey, // track background
		//	"ScrollBar.foreground", Color.blue,
			"ScrollBar.darkShadow", lightGrey,

		//	"TabbedPane.tabAreaBackground", new Color(0xFF0000), // doesnt seem to do much
			"TabbedPane.selected", new Color(0x462F51), // selected tab
	//		"TabbedPane.selectHighlight", lightGrey, // selected tab border
			"TabbedPane.background", new Color(0x5E476A), // other tabs
			"TabbedPane.foreground", Color.WHITE, // text color
		//	"TabbedPane.highlight", Color.RED, // nothing
		//	"TabbedPane.light", Color.RED, // other tab border
	//		"TabbedPane.darkShadow", Color.WHITE, // all tab borders
			"TabbedPane.font", new Font("Verdana", Font.PLAIN, 11),

		//	"SplitPane.dividerSize", new Integer(10),

			"MenuItem.background", vLightGrey,
			"Menu.background", vLightGrey,
		//	"MenuItem.border", new LineBorder(lightGrey),
		//	"Menu.border", lightGrey,
			"PopupMenu.border", new LineBorder(lightGrey),
			"MenuItem.selectionBackground", lightGrey,
			"Menu.selectionBackground", lightGrey,

			"SelectionTool.background", lightGrey, // matrix specific
			"SelectionTool.bordercolor", lightGrey, // matrix specific

			"InspectorNavigator.background", vLightGrey,

			"MenuItem.font", PLAIN_FONT_10,
			"Menu.font", PLAIN_FONT_10,

			"CueLine.stroke", Color.BLACK,

		};
		table.putDefaults(defaults);
	}

	protected void initSystemColorDefaults(UIDefaults table) {
		super.initSystemColorDefaults(table);

		Object[] systemColors = {
			"scrollbar", lightGrey,
		//	"control", vLightGrey,
		//	"controlDkShadow", vLightGrey,  // divider dots
		//	"controlHighlight", vLightGrey,
		};

		for (int i = 0; i < systemColors.length; i += 2) {
			table.put((String) systemColors[i], systemColors[i + 1]);
		}
	}
}
