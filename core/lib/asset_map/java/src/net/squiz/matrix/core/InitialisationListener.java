
package net.squiz.matrix.core;

import java.util.EventListener;

public interface InitialisationListener extends EventListener {
	public void initialisationComplete(InitialisationEvent evt);
}
