
package net.squiz.matrix.assetmap;

import net.squiz.matrix.matrixtree.*;
import javax.swing.*;

public interface View {
	public MatrixTree getTree();
	public String getName();
	public void setName(String name);
	public JComponent getViewComponent();
}
