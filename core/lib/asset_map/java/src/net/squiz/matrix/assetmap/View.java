
package net.squiz.matrix.assetmap;

import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.inspector.*;
import javax.swing.*;

public interface View {
	public MatrixTree getTree();
	public Inspector getInspector();
	public String getName();
	public void setName(String name);
	public JComponent getViewComponent();
}
