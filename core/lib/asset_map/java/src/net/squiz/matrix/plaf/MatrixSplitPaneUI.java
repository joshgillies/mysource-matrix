
package net.squiz.matrix.plaf;

import javax.swing.plaf.metal.*;
import javax.swing.plaf.basic.*;
import net.squiz.matrix.inspector.*;

/**
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixSplitPaneUI extends BasicSplitPaneUI {
	
	
	public MatrixSplitPaneUI() {
	}
	
	public BasicSplitPaneDivider createDefaultDivider() {
		return new MatrixSplitPaneDivider(this);
	}
}
