
package net.squiz.matrix.plaf;

import javax.swing.plaf.metal.*;
import java.awt.Insets;

/**
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class VerticalTabbedPaneUI extends MetalTabbedPaneUI {
	private boolean tabsVertical;

	public VerticalTabbedPaneUI(boolean tabsVertical) {
		this.tabsVertical = tabsVertical;
	}

	protected void installDefaults() {
		super.installDefaults();
		tabAreaInsets = new Insets(0, 0, 0, 0);
		if (tabsVertical) {
			tabInsets = new Insets(0, 1, 0, 1);
			selectedTabPadInsets = new Insets(0, 1, 0, 1);
			// MM: put this somewhere higher
			contentBorderInsets = new Insets(0, 0, 0, 0);
			tabAreaInsets = new Insets(0, 0, 0, 0);
		}
	}
}
