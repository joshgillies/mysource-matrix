<?php
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
* $Id: event_manager.inc,v 1.36 2012/08/30 01:09:19 ewang Exp $
*
*/


/**
* Event_Manager
*
* Purpose
*    Handles matching an event listener with a event broadcast request.
*    Two Types of events can be listening/broadcasted:
*
*    1) Static Events - Events that are defined and registered at install time. These are assets that need to supply
*       additional information about their relationship with the broadcaster (type_code, is_dependant, is_exclusive, link_types,
*       link values etc).
*
*    2) Runtime Events - Events that are defined and registered at runtime. These are events that are not limited to assets,
*       and do not require to define any information about their broadcaster (anyone can listen for this event).
*
* Example for static events (in the management class of that asset):
*
* <PRE>
* function getEventList()
*	{
*		return Array(
*				Array(
*					'event_name'                    => 'contentsUpdated',
*					'broadcast_type_code'           => 'bodycopy_container',
*					'broadcast_strict_type_code'    => false,	// div, table cell etc
*					'options'	=> Array(
*						'side_of_link'      => 'minor',	// side of link where the broadcaster exists
*						'indirect'          => true,
*						'is_exclusive'      => null,
*						'is_dependant'      => 1,
*						'value'             => '',
*						'link_type'         => SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
*					),
*				),
*		);
*	}//end getEventList()
* </PRE>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.36 $
* @package MySource_Matrix
*/
class Event_Manager extends MySource_Object
{

	/**
	* @var Array(mixed)
	* Caching of event listeners
	*/
	var $_runtime_event_listeners = Array();

	/**
	* @var Array(&object Asset)
	* The listener objects
	*/
	var $_runtime_listener_objects = Array();

	/**
	* Constructor
	*
	*/
	function Event_Manager()
	{
		parent::MySource_Object();

	}//end constructor


	/**
	* Allows an object to listen to a runtime event
	*
	* If the listener does not want to receive broadcasts from events that are currently polling, set the
	* broadcast_polling_events to FALSE.
	*
	* @param object		&$object					the object that is listening (use $this for yourself)
	* @param array		$events						an array of events that the object wants to listen to
	* @param boolean	$broadcast_polling_events	if TRUE any events that are polling will be broadcasted to this listener
	*
	* @return boolean
	* @access public
	*/
	function addEventListener(&$listener, $events=Array(), $broadcast_polling_events=true)
	{
		if (empty($events)) return false;

		foreach ($events as $event_name) {
			$event_name = trim($event_name);
			if (empty($event_name)) continue;

			if (!array_key_exists($event_name, $this->_runtime_event_listeners)) {
				$this->_runtime_event_listeners[$event_name] = Array();
			}
			$index = count($this->_runtime_listener_objects);
			$this->_runtime_listener_objects[] = $listener;
			$this->_runtime_event_listeners[$event_name][$index] = true;
		}
		return true;

	}//end addEventListener()


	/**
	* Broadcast events to the listeners that we added at runtime
	*
	* @param &object Asset	$broadcaster	the broadcaster of this event
	* @param string			$event_name		the event name being broadcasted
	* @param Array			$vars			vars to be past to the listener
	*
	* @access private
	* @return boolean
	*/
	function _broadcastRuntimeEvents(&$broadcaster, $event_name, $vars=Array())
	{
		if (!isset($this->_runtime_event_listeners[$event_name])) {
			return false;
		}

		$event_name = trim($event_name);
		$function_name = 'on'.$event_name;

		foreach (array_keys($this->_runtime_event_listeners[$event_name]) as $index) {
			if (!isset($this->_runtime_listener_objects[$index])) {
				continue;
			}
			$listener = $this->_runtime_listener_objects[$index];
			if (is_null($listener)) continue;
			if (!method_exists($listener, $function_name)) {
				continue;
			}
			$listener->$function_name($broadcaster, $vars);
		}

		return true;

	}//end _broadcastRuntimeEvents()


	/**
	* To call an event function on an object that that has signed up to hear a particular event
	*
	* eg.
	* <PRE>
	* $em = $GLOBALS['SQ_SYSTEM']->getEventManager();
	* $em->broadcastEvent($asset, 'eventName', Array('assetid' => $asset->id));
	* </PRE>
	*
	* @param &object Asset	$asset			the asset that triggered the event
	* @param string			$event_name		the name of the event that the asset is triggering
	* @param Array			$vars			any information that the event want to tell the listener
	*
	* @access public
	* @return boolean
	*/
	function broadcastEvent(&$broadcaster, $event_name='', $vars=Array())
	{

		// note: event names should be cammel topped (first letter of each word is a capital, excluding the first word).
		// the listener needs to have a function of the event name prepended with 'on'
		// eg. for an event name contentsUpdated the listener needs a function called onContentsUpdated()
		// The function needs to accept an array in the arguments list (for vars)

		// check the runtime events first
		$this->_broadcastRuntimeEvents($broadcaster, $event_name, $vars);

		// create a temp entry for the event listeners (runtime wide as event_manager is cached)
		if (!isset($this->_tmp['event_listeners'])) {
			$this->_loadCachedEventListeners();
		}

		// we don't have an entry for this event
		if (!isset($this->_tmp['event_listeners']['global'][$event_name]) && !isset($this->_tmp['event_listeners'][strtolower(get_class($broadcaster))][$event_name])) {
			return false;
		}

		$listeners = Array();
		$listener_codes = Array();

		if (isset($this->_tmp['event_listeners']['global'][$event_name])) {
			$listener_codes += $this->_tmp['event_listeners']['global'][$event_name];
		}
		if (isset($this->_tmp['event_listeners'][get_class_lower($broadcaster)][$event_name])) {
			$listener_codes += $this->_tmp['event_listeners'][get_class_lower($broadcaster)][$event_name];
		}

		/**
		 * These will get filled in as necessary
		 * (once per broadcast event)
		 * Only need to fill them in if we need the data,
		 * otherwise we could do it at the top here.
		 */
		$possible_listeners_parents = array();
		$possible_listeners_children = array();

		foreach ($listener_codes as $listener_type_code => $options) {
			// these $options never seem to be set, force them to be set to avoid notices from PHP
			if (empty($options['type_code'])) {
				$options['type_code'] = '';
			}
			if (empty($options['strict_type_code'])) {
				$options['strict_type_code'] = false;
			}
			if (empty($options['global'])) {
				$options['global'] = false;
			}

			$side_of_link = (isset($options['side_of_link']) && ($options['side_of_link'] == 'minor')) ? 'major' : 'minor';

			if ($options['global']) {
				// Global: all global events should be heard even if outside the chain
				$assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($listener_type_code, true);
			} else if ($options['indirect']) {

				// no point continuing if this object does not inherit from asset
				if (!($broadcaster instanceof asset)) return true;
				if (!$broadcaster->id) return;

				// let's not query the tree unless we have to
				if ($options['is_dependant'] && !$options['is_exclusive']) {
					if (isset($found_listener)) {
						continue;
					}

					// do this until one of the returned assets matches a listener type code
					if ($side_of_link === 'minor') {
						if (empty($possible_listeners_parents)) {
							$possible_listeners_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($broadcaster->id);
						}
						$possible_listeners = $possible_listeners_parents;
					} else {
						if (empty($possible_listeners_children)) {
							$possible_listeners_children = $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($broadcaster->id);
						}
						$possible_listeners = $possible_listeners_children;
					}

					// get type codes of our assetids (only need to do it once)
					if (!isset($types)) {
						for ($i=0; $i<count($possible_listeners); $i++) {
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($possible_listeners[$i]);
							$types[$possible_listeners[$i]] = $asset->type();
						}
					}

				} else if ($options['is_dependant'] || $options['is_exclusive'] || $options['type_code']) {
					$assetids = $this->_getDiscreteAssets($broadcaster->id, $listener_type_code, $side_of_link, $options);
				} else {

					// this is in the reverse to what is normal.
					// eg. if a form wants to listen to events broadcasted from a bodycopy (which is a child of the form)
					// then the form wants to hear from minor. When the bodycopy broadcasts, the form is on the major of
					// the bodycopy, comprende?

					$function = 'get'.(($options['side_of_link'] == 'minor') ? 'Parents' : 'Children');
					$assetids = $GLOBALS['SQ_SYSTEM']->am->$function($broadcaster->id, $listener_type_code, $options['strict_type_code']);
					if (empty($assetids)) continue;
				}

			} else {

				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($broadcaster->id, $options['link_type'], $listener_type_code, true, $options['side_of_link'], $options['value'], $options['is_dependant'], $options['is_exclusive']);
				if (empty($links)) continue;

				$assetids = Array();
				foreach ($links as $link) {
					$assetids[] = $link[$side_of_link.'id'];
				}

			}// end if

			// are we filtering our listeners?
			if (isset($types)) {
				// get our listener
				if (in_array($listener_type_code, $types)) {
					foreach ($types as $assetid => $type_code) {
						if ($type_code === $listener_type_code) {
							$found_listener = TRUE;
							$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
							if (is_null($asset)) continue;
							$listeners[] = $asset;
						}
					}
				}
			} else {
				foreach ($assetids as $assetid) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (is_null($asset)) continue;
					$listeners[] = $asset;
				}
			}

		}// end foreach

		if (empty($listeners)) return false;

		$function = 'on'.$event_name;
		for (reset($listeners); null !== ($key = key($listeners)); next($listeners)) {
			$listener = $listeners[$key];
			if (!method_exists($listener, $function)) {
				trigger_localised_error('SYS0127', sprintf(translate('Unable to find event callback function "%1$s" in Asset "%2$s" (#%3$s)'), $function, $listener->type(), $asset->id), E_USER_WARNING);
				return false;
			}
			$listener->$function($broadcaster, $vars);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($listener);
		}// end for

		return true;

	}//end broadcastEvent()


	/**
	* Traverse up or down the tree for asset information
	*
	* Collects all assets of the wanted type_code
	*
	* @param integer	$assetid		the assetid of the broadcaster
	* @param string		$type_code		the type_code of the listener
	* @param string		$side_of_link	the size of link where the broadcaster exists
	* @param array		$info			some information about the discretion of assets to return
	*
	* @access public
	* @return Array(integer) assetid
	*/
	function _getDiscreteAssets($assetid='', $type_code='', $side_of_link='minor', $info=Array())
	{
		if (empty($assetid) || empty($type_code)) {
			return Array();
		}

		$sql = 'SELECT (t2.treeid || \'%\')
				FROM sq_ast_lnk l1
				  INNER JOIN sq_ast_lnk_tree t1 ON l1.linkid = t1.linkid,
				  sq_ast_lnk l2
				  INNER JOIN sq_ast_lnk_tree t2 ON l2.linkid = t2.linkid,
				  sq_ast a
				WHERE l1.'.$side_of_link.'id = :assetid
				  AND SUBSTR(t1.treeid, 1, LENGTH(t2.treeid)) = t2.treeid
				  AND a.assetid=l2.'.$side_of_link.'id
				  AND a.type_code= :type_code
				ORDER BY t2.treeid DESC';

		$result = NULL;
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			MatrixDAL::bindValueToPdo($query, 'type_code', $type_code);
			$result = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get Discrete Assets due to database error: '.$e->getMessage());
		}

		if (empty($result)) return Array();

		$treeid_string = '';
		foreach ($result as $treeid) {
			$treeid_string .= 't2.treeid LIKE \''.$treeid.'\' OR ';
		}
		$treeid_string = substr($treeid_string, 0, -4);


		$sql = 'SELECT a.assetid, t2.treeid, a.type_code, l2.link_type, l2.is_dependant, l2.is_exclusive
				FROM sq_ast_lnk l1
				  INNER JOIN sq_ast_lnk_tree t1 ON l1.linkid = t1.linkid,
				  sq_ast_lnk l2
				  INNER JOIN sq_ast_lnk_tree t2 ON l2.linkid = t2.linkid,
				  sq_ast a
				WHERE l1.'.$side_of_link.'id = :assetid
					AND SUBSTR(t1.treeid, 1, LENGTH(t2.treeid)) = t2.treeid
					AND a.assetid = l2.'.$side_of_link.'id
					AND ('.$treeid_string.')
				ORDER BY t2.treeid';

		$result = NULL;

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get Discrete Assets due to database error: '.$e->getMessage());
		}//end try

		$assetids = Array();
		$start_treeid = true;
		$popped_treeid = false;

		for ($i = 0; $i < count($result); $i++) {

			if (isset($result[$i+1]) && strpos($result[$i+1]['treeid'], $result[$i]['treeid']) === false) {

				// the next treeid is not based on our treeid, so we will be starting a new branch
				$start_treeid = true;
				$popped_treeid = false;

			} else {

				if (!$popped_treeid) {

					if (in_array($result[$i]['assetid'], $assetids)) {
						// we have already approved this asset to get notified
						// so lets skip the rest of the branch
						$popped_treeid = true;
						continue;
					}

					if ($side_of_link == 'minor') {
						if (!isset($result[$i +1])) continue;
						$test_info = $result[$i +1];
					} else {
						$test_info = $result[$i];
					}

					$valid_assetid = true;
					if ($valid_assetid && $info['value'] && $info['value'] != $test_info['value']) {
						$valid_assetid = false;
					}
					if ($valid_assetid && !$test_info['is_dependant'] && $info['is_dependant']) {
						$valid_assetid = false;
					}
					if ($valid_assetid && !$test_info['is_exclusive'] && $info['is_exclusive']) {
						$valid_assetid = false;
					}
					if ($valid_assetid &&  $test_info['link_type'] &  $info['link_type'] <= 0) {
						$valid_assetid = false;
					}

					if ($start_treeid && $valid_assetid) {
						$assetids[] = $result[$i]['assetid'];
						$start_treeid = false;
					} else if (!$start_treeid && !$valid_assetid) {
						// the last assetid we added failed the test, so get rid of it
						array_pop($assetids);
						$popped_treeid = true;
					}

				}//end if !$popped_treeid

			}//end if not starting again next time around

		}//end for

		return $assetids;

	}//end _getDiscreteAssets()


	/**
	* Write the cache file of the current static events in the system
	*
	* This occurs after all the packages have written their listeners to the system
	*
	* @return boolean
	* @access public
	*/
	function writeStaticEventsCacheFile()
	{
		if (!isset($this->_tmp['event_listeners']) || empty($this->_tmp['event_listeners'])) {
			return false;
		}

		$output = '<'.'?php'."\n".' $cached_event_listeners_array = ';
		$output .= var_export($this->_tmp['event_listeners'], true);
		$output .= "\n?".'>';

		if (!string_to_file($output, SQ_DATA_PATH.'/private/events/event_listeners.inc')) {
			return false;
		}
		unset($this->_tmp['event_listeners']);

		return true;

	}//end writeStaticEventsCacheFile()


	/**
	* Add static event listeners
	*
	* This function gets called from package manager of each package that has any management classes that have
	* events to be installed
	*
	* @param array		$listeners				array of listeners
	* @param string		$listener_type_code		the type code of the listener
	* @param boolean	$recursive_call			if TRUE this call is recursive
	*
	* @access private
	* @return boolean
	*/
	function installStaticEvents($listeners=Array(), $listener_type_code='', $recursive_call=false)
	{
		if (!is_array($listeners)|| empty($listeners)) {
			return false;
		}
		if (!isset($this->_tmp['event_listeners'])) {
			$this->_loadCachedEventListeners();
		}

		$listener_name = Array();
		foreach ($listeners as $index => $info) {
			if (!isset($this->_tmp['event_listeners'][$info['broadcast_type_code']])) {
				$this->_tmp['event_listeners'][$info['broadcast_type_code']] = Array();
			}
			if (!isset($this->_tmp['event_listeners'][$info['broadcast_type_code']][$info['event_name']])) {
				$this->_tmp['event_listeners'][$info['broadcast_type_code']][$info['event_name']] = Array();
			}

			// if they have been lazy, do some cleanup
			if (!isset($info['options']['value'])) {
				$info['options']['value'] = '';
			}

			if (!is_array($info['options'])) {
				trigger_localised_error('SYS0143', sprintf(translate('Expecting Array for event options for "%1$s", got %2$s'), $listener_type_code, gettype($info['options'])), E_USER_WARNING);
				return false;
			}

			$this->_tmp['event_listeners'][$info['broadcast_type_code']][$info['event_name']][$listener_type_code] = $info['options'];

			// if the broadcast type code can be of multiple types, then we have to add some more
			// entries for these in the array
			if (!$info['broadcast_strict_type_code'] && !$recursive_call) {
				// only broadcast if the listener is installed, fail silently
				if ($GLOBALS['SQ_SYSTEM']->am->installed($info['broadcast_type_code'])) {
					$type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($info['broadcast_type_code']);
					if (empty($type_codes)) continue;
					foreach ($type_codes as $type_code) {
						$info['broadcast_type_code'] = $type_code;
						$this->installStaticEvents(Array($info), $listener_type_code, true);
					}
				}
			}
			$listener_name[] = $info['event_name'];
		}

		// done with everything ?
		// remove any of the listeners that might have been deleted from our asset
		foreach ($this->_tmp['event_listeners'] as $broadcast_type_code => $listener_info) {
			foreach ($listener_info as $name => $asset_array) {
				foreach ($asset_array as $asset_type => $info) {
					if ($asset_type == $listener_type_code && !in_array($name, $listener_name)) {
						unset($this->_tmp['event_listeners'][$broadcast_type_code][$name][$asset_type]);
					}
				}
			}
		}

		return true;

	}//end installStaticEvents()


	/**
	* Load event listeners from the cache file to our _tmp array
	*
	* @access private
	* @return void
	*/
	function _loadCachedEventListeners()
	{
		if (!file_exists(SQ_DATA_PATH.'/private/events/event_listeners.inc')) {
			$this->_tmp['event_listeners'] = Array();
			return;
		}
		require SQ_DATA_PATH.'/private/events/event_listeners.inc';
		if (!isset($cached_event_listeners_array)) {
			trigger_error('Failed loading event listeners from SQ_DATA_PATH/private/events/event_listeners.inc; Event listeners file may be corrupt', E_USER_ERROR);
			return;
		}
		$this->_tmp['event_listeners'] = $cached_event_listeners_array;

	}//end _loadCachedEventListeners()


}//end class
?>
