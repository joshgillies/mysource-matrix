
package net.squiz.matrix.ui;

import java.awt.Point;


public interface SelectionHandler {
	public boolean canSelect(Point point);
	public void updateSelection(Point point);
	public void clearSelection();
}
