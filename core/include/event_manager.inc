<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: event_manager.inc,v 1.7 2003/11/26 00:51:10 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Event_Manager
*
* Purpose 
*    Handles matching an event listener with a event broadcast request.
*    Two Types of events can be listening/broadcasted:
*
*    1) Static Events - Events that are defined and registered at install time. These are assets that need to supply
*       additional information about their relationship with the broadcaster (type_code, dependant, exclusive, link_types, 
*       link values etc).
*       
*    2) Runtime Events - Events that are defined and registered at runtime. These are events that are not limited to assets, 
*       and do not require to define any information about their broadcaster (anyone can listen for this event).
*       Runtime events are polled to ensure that if a event is broadcasted before the listener has registered to 
*       listen for the event, it can be notified about it regardless. The listener can choose not to receive a call 
*       for polled events.
*
* Example for static events (in the management class of that asset):
*
* <PRE>
* function getEventList()
*	{
*		return Array(
*				Array(
*					'event_name'				 => 'contentsUpdated',
*					'broadcast_type_code'		 => 'bodycopy_container',
*					'broadcast_strict_type_code'	=> false,	// div, table cell etc
*					'options'	=> Array(
*						'side_of_link'					=> 'minor',	// side of link where the broadcaster exists
*						'indirect'						=> true,
*						'exclusive'						=> null,
*						'dependant'						=> 1,
*						'value'							=> '',
*						'link_type'						=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
*					),
*				),
*		);
*	}//end getEventList()
* </PRE>
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
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
	* @var Array(mixed)
	* Events that are pending broadcast (runtime)
	*/
	var $_polling_broadcast_events = Array();


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

			// if there are some events polling for this listener, and it has not explicity denied polling events,
			// then broadcast to this listener
			
			if (isset($this->_polling_broadcast_events[$event_name]) && $broadcast_polling_events) {
				$this->_broadcastPollingEvent($listener, $event_name);
			}
			if (!in_array($event_name, $this->_runtime_event_listeners)) {
				$this->_runtime_event_listeners[$event_name] = Array();
			}
			$index = count($this->_runtime_listener_objects);
			$this->_runtime_listener_objects[] = &$listener;
			$this->_runtime_event_listeners[$event_name][$index] = true;
		}
		return true;

	}//end addEventListener()


	/**
	* Broadcast an event that has been polling
	*
	* All broadcasts of events are polling at one stage, but this function will only get called if an event listener is added
	* after the original broadcast of the event.
	*
	* @param &object mixed	$listener			the object listening to the event
	* @param string			$event_name			the name of the event to broadcast
	*
	* @access private
	* @return boolean
	*/
	function _broadcastPollingEvent(&$listener, $event_name)
	{
		if (!isset($this->_polling_broadcast_events[$event_name])) return false;
		$function_name = 'on'.$event_name;

		// lets just hope that there is something in the vars that distinguishes the difference between
		// each of the polling broadcasters
		foreach ($this->_polling_broadcast_events[$event_name] as $vars) {
			if (!method_exists($listener, $function_name)) continue;
			$listener->$function_name($vars);
		}
		return true;
	
	}//end _broadcastPollingEvent()


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
		// if there are no events registered for this broadcast, or someone else other than those currently registered for
		// this event wants to here it but is not currently registered, then it is possible that 
		// there is a runtime instantiation issue (broadcaster got instanciated before the listener had a chance)
		// to register for an event, so poll this event.
		
		// we have to use the class name instead of type_code, because not all broadcasters are assets
		if (!isset($this->_polling_broadcast_events[$event_name][get_class($broadcaster)])) {
			if (!isset($this->_polling_broadcast_events[$event_name])) {
				$this->_polling_broadcast_events[$event_name] = Array();
			}
			$this->_polling_broadcast_events[$event_name][get_class($broadcaster)] = $vars;
		}
		if (!isset($this->_runtime_event_listeners[$event_name])) return false;
		
		$function_name = 'on'.$event_name;
		$event_name = trim($event_name);
		
		foreach (array_keys($this->_runtime_event_listeners[$event_name]) as $index) {
			if (!isset($this->_runtime_listener_objects[$index])) continue;
			$listener =& $this->_runtime_listener_objects[$index];
			if (is_null($listener)) continue;
			if (!method_exists($listener, $function_name)) continue;

			// add some extra info to the vars if they need to know about the broadcaster, if it inherits from asset
			if (is_a($broadcaster, 'asset')) {
				$vars['broadcaster_assetid'] = $broadcaster->id;
				$vars['broadcaster_type_code'] = $broadcaster->type();
			}

			$listener->$function_name($vars);
		} // end foreach

		return true;

	}//end _broadcastRuntimeEvents()


	/**
	* To call an event function on an object that that has signed up to hear a particular event
	*
	* eg.
	* <PRE>
	* $em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
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

		// no point continuing if this object does not inherit from asset
		if (!is_a($broadcaster, 'asset')) return true;
		
		// create a temp entry for the event listeners (runtime wide as event_manager is cached)
		if (!isset($this->_tmp['event_listeners'])) {
			if (!file_exists(SQ_DATA_PATH.'/private/events/event_listeners.inc')) return false;
			
			require_once SQ_DATA_PATH.'/private/events/event_listeners.inc';
			if (!isset($cached_event_listeners_array)) return false;
			$this->_tmp['event_listeners'] = $cached_event_listeners_array;
		}

		// if we don't have an entry for this event, then g.t.f.o
		if (!isset($this->_tmp['event_listeners'][$broadcaster->type()][$event_name])) return false;
		$listeners = Array();

		foreach ($this->_tmp['event_listeners'][$broadcaster->type()][$event_name] as $listener_type_code => $options) {
			$side_of_link = ($options['side_of_link'] == 'minor') ? 'major' : 'minor';
			
			if ($options['indirect']) {
				
				if ($options['dependant'] || $options['exclusive'] || $options['type_code']) {
					$assetids = $this->_getDiscreteAssets($broadcaster->id, $listener_type_code, $side_of_link, $options);
				} else {

					// this is in the reverse to what is normal.
					// eg. if a form wants to listen to events broadcasted from a bodycopy (which is a child of the form)
					// then the form wants to hear from minor. When the bodycopy broadcasts, the form is on the major of
					// the bodycopy, comprende?
					
					$function = 'get'.(($options['side_of_link'] == 'minor') ? 'Parents' : 'Children');
					$assetids = $GLOBALS['SQ_SYSTEM']->am->$function($broadcaster->id, $listener_type_code, $options['strict_type_code']);
					if (empty($assetids)) return false;
				}

			} else {
				$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($broadcaster->id, $options['link_type'], $listener_type_code, true, $options['side_of_link'], $options['value'], $options['dependant'], $options['exclusive']);
				if (empty($links)) return false;

				$assetids = Array();
				foreach ($links as $link) $assetids[] = $link[$side_of_link.'id'];
			}// end if

			foreach ($assetids as $assetid) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;
				$listeners[] =& $asset;
			}

		}// end foreach

		if (empty($listeners)) return false;

		for (reset($listeners); null !== ($key = key($listeners)); next($listeners)) {
			$listener =& $listeners[$key];
			$function = 'on'.ucfirst($event_name);

			if (!method_exists($listener, $function)) {
				trigger_error('Could not find event callback function "'.$function.'" in Asset "'.$listener->type().'" (#'.$asset->id.')', E_USER_WARNING);
				return false;
			}
			$listener->$function($vars);
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
		if (empty($assetid) || empty($type_code)) return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(t2.treeid, '.$db->quote('%').')' : '(t2.treeid || '.$db->quote('%').')';
		
		// big f.o query
		$subs = Array('SELECT '.$concat.' FROM '
		.SQ_TABLE_PREFIX.'asset_link l1 INNER JOIN '.SQ_TABLE_PREFIX.'asset_link_tree t1 ON l1.linkid = t1.linkid, '
		.SQ_TABLE_PREFIX.'asset_link l2 INNER JOIN '.SQ_TABLE_PREFIX.'asset_link_tree t2 ON l2.linkid = t2.linkid, '
		.SQ_TABLE_PREFIX.'asset a '
		.'WHERE l1.'.$side_of_link.'id = '.$db->quote($assetid).' '
		.'AND SUBSTRING(t1.treeid FROM 1 FOR CHAR_LENGTH(t2.treeid)) = t2.treeid '
		.'AND a.assetid=l2.'.$side_of_link.'id '
		.'AND a.type_code='.$db->quote($type_code).' ORDER BY t2.treeid DESC LIMIT 1');

		$where =' l1.'.$side_of_link.'id = '.$db->quote($assetid).'
			 AND SUBSTRING(t1.treeid FROM 1 FOR CHAR_LENGTH(t2.treeid)) = t2.treeid 
			 AND a.assetid=l2.'.$side_of_link.'id and t2.treeid like (~SQ0~) ORDER BY t2.treeid';

		$where = db_extras_subquery($db, $where, $subs);

		$sql = 'SELECT a.assetid, t2.treeid, a.type_code, l2.link_type, l2.dependant, l2.exclusive '
			.'FROM '
			.SQ_TABLE_PREFIX.'asset_link l1 INNER JOIN '.SQ_TABLE_PREFIX.'asset_link_tree t1 ON l1.linkid = t1.linkid, '
			.SQ_TABLE_PREFIX.'asset_link l2 INNER JOIN '.SQ_TABLE_PREFIX.'asset_link_tree t2 ON l2.linkid = t2.linkid, '
			.SQ_TABLE_PREFIX.'asset a WHERE '.$where;

		$result = $db->getAll($sql);

		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$assetid = Array();
		$nice_value = true;

		for ($i = 0; $i < count($result); $i++) {
			if ($info['value'] && $info['value'] != $result[$i]['value']) $nice_value = false;
			if (!$result[$i]['dependant'] && $info['dependant'])    continue;
			if (!$result[$i]['exclusive'] && $info['exclusive'])    continue;
			if ($result[$i]['link_type'] & $info['link_type'] <= 0) continue;
			if ($i == 0) $assetid[] = $result[$i]['assetid'];
		}
		if (!$nice_value) return Array();
		return $assetid;

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
		if (!isset($this->_tmp['event_listeners']) || empty($this->_tmp['event_listeners'])) return false;
		
		$output = '<'.'?php'."\n".' $cached_event_listeners_array = ';
		$output .= var_export($this->_tmp['event_listeners'], true);
		$output .= "\n?".">";
		
		if (!string_to_file($output, SQ_DATA_PATH.'/private/events/event_listeners.inc')) return false;
		unset($this->_tmp['event_listeners']);
		
		return true;

	}//end writeStaticEventsCacheFile()


	/**
	* Add static event listeners
	*
	* This function gets called from package manager of each package that has any management classes that have 
	* events to be installed
	* 
	* @param Array		$listeners				array of listeners
	* @param string		$listener_type_code		the type code of the listener
	* @param boolean	$recursive_call			if TRUE this call is recursive
	*
	* @access private
	* @return boolean
	*/
	function installStaticEvents($listeners=Array(), $listener_type_code='', $recursive_call=false)
	{
		if (!is_array($listeners)|| empty($listeners)) return false;
		if (!isset($this->_tmp['event_listeners'])) $this->_tmp['event_listeners'] = Array();

		foreach ($listeners as $info) {
			
			if (!isset($this->_tmp['event_listeners'][$info['broadcast_type_code']])) {
				$this->_tmp['event_listeners'][$info['broadcast_type_code']] = Array();
			}
			if (!isset($this->_tmp['event_listeners'][$info['broadcast_type_code']][$info['event_name']])) {
				$this->_tmp['event_listeners'][$info['broadcast_type_code']][$info['event_name']] = Array();
			}

			// if they have been lazy, do some cleanup
			if (!isset($info['options']['value'])) $info['options']['value'] = '';

			if (!is_array($info['options'])) {
				trigger_error('Expecting Array for event options for "'.$listener_type_code.'", got '.gettype($info['options']), E_USER_WARNING);
				return false;
			}
				
			$this->_tmp['event_listeners'][$info['broadcast_type_code']][$info['event_name']][$listener_type_code] = $info['options'];

			// if the broadcast type code can be of multiple types, then we have to add some more
			// entries for these in the array
			
			if (!$info['broadcast_strict_type_code'] && !$recursive_call) {
				$type_codes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($info['broadcast_type_code']);
				if (empty($type_codes)) continue;
				foreach ($type_codes as $type_code) {
					$info['broadcast_type_code'] = $type_code;
					$this->installStaticEvents(Array($info), $listener_type_code, true);
				}
			}
		}
		return true;

	}//end installStaticEvents()


}//end class

?>