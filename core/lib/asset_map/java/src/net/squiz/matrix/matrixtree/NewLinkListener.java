
package net.squiz.matrix.matrixtree;
import java.util.EventListener;

public interface NewLinkListener extends EventListener {
	public void requestForNewLink(NewLinkEvent evt);
}
