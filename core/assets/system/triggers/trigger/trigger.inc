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
* $Id: trigger.inc,v 1.3 2006/04/03 00:12:11 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Trigger
*
* Purpose
*	Represents a trigger. Used for backend edit interface
*	purposes only. All other related trigger functions remain
*	in the Trigger_Manager.
*
* @author Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*
* @see Trigger_Manager
*/
class Trigger extends Asset
{


	/**
	* The ID of the trigger manager
	*
	* @var string
	* @access public
	*/
	var $tmid = 0;


	/**
	* Constructor
	*
	* @param string	$timd	The ID of the Trigger Manager
	* @param string	$id		The ID of this Trigger
	*
	* @return void
	* @access public
	*/
	function Trigger($tmid=0, $id=0)
	{
		$this->_ser_attrs = TRUE;
		$this->_loadVars();

		// attempt to load trigger from database
		if (!empty($tmid) && !empty($id)) {
			$this->load($tmid, $id);
		}

	}//end constructor


	/**
	* Sets the trigger_manager that this trigger belongs to
	*
	* @param int	$tmid	The id of the trigger_manager
	*
	* @return boolean
	* @access private
	*/
	function setTriggerManager($tmid)
	{
		if (empty($tmid)) return FALSE;

		$tm =& $GLOBALS['SQ_SYSTEM']->am->getAsset($tmid, 'trigger_manager');
		if (!is_null($tm)) {
			// this is our new trigger manager
			$this->tmid = $tmid;
			return TRUE;
		}
		return FALSE;

	}//end setTriggerManager()


	/**
	* Gets the trigger_manager that this trigger belongs to
	*
	* @return object
	* @access public
	*/
	function &getTriggerManager()
	{
		$asset = NULL;
		if (isset($this->tmid) && !empty($this->tmid)) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->tmid, 'trigger_manager');
		}
		return $asset;

	}//end getTriggerManager()


	/**
	* Loads this object from the database
	*
	* @param int	$tmid	The id of the trigger manager
	* @param int	$id		The id of the trigger
	*
	* @return boolean
	* @access private
	*/
	function load($tmid, $id)
	{
		// is our trigger manager the real thing?
		if (!$this->setTriggerManager($tmid)) return FALSE;
		$tm =& $this->getTriggerManager();

		// we use the trigger manager function to load the trigger
		$trigger_info = $tm->_loadTrigger($id);
		if (empty($trigger_info)) return FALSE;

		// trigger exists, so the id is valid
		$this->id = implode(':', Array($tmid, $id));

		$this->name = $trigger_info['name'];
		$this->short_name = $trigger_info['name'];

		// mimic the bridge status
		$this->status = $tm->status;

		// load attributes
		$this->vars['name']['value']		= $trigger_info['name'];
		$this->vars['description']['value']	= $trigger_info['description'];
		$this->vars['events']['value']		= $trigger_info['data']['events'];
		$this->vars['conditions']['value']	= $trigger_info['data']['conditions'];
		$this->vars['actions']['value']		= $trigger_info['data']['actions'];
		$this->vars['active']['value']		= $trigger_info['active'];
		$this->vars['blocking']['value']	= isset($trigger_info['data']['settings']['blocking']) ? $trigger_info['data']['settings']['blocking'] : FALSE;

		return TRUE;

	}//end load()


	/**
	* Create this asset
	*
	* The return value will be:<br/>
	* <ul>
	*   <li>FALSE if the asset was not created</li>
	*   <li>the ID of the newly created link if the asset and intital link were created</li>
	* </ul>
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </pre>
	*
	* @return mixed int|boolean
	* @access public
	*/
	function create(&$link)
	{
		$save_result = $this->saveAttributes();
		if (!$save_result) return $save_result;

		// possibly not the best way to do this, but we need
		// to set our status / name and the trigger manager id
		$id = end(explode(':', $this->id));
		$load_result = $this->load($this->tmid, $id);

		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
		$tm =& $this->getTriggerManager();

		// hopefully the asset map is listening
		$em->broadcastEvent($tm, 'CreateLink', Array('linkid' => $this->id));

		return $save_result && $load_result;

	}//end create()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		$trigger_info = Array();

		if (empty($this->tmid)) return FALSE;
		$id_parts = explode(':', $this->id);

		// if we supply an id to the trigger manger, existing
		// trigger will be updated
		$trigger_id = array_get_index($id_parts, 1);
		if (!is_null($trigger_id)) {
			$trigger_info['id'] = $trigger_id;
		}

		// build trigger info from attributes
		$trigger_info['name']							= $this->attr('name'); 
		$trigger_info['description']					= $this->attr('description');
		$trigger_info['data']['events']					= $this->attr('events');
		$trigger_info['data']['conditions']				= $this->attr('conditions');
		$trigger_info['data']['actions']				= $this->attr('actions');
		$trigger_info['active']							= $this->attr('active');
		$trigger_info['data']['settings']['blocking']	= $this->attr('blocking');

		// do not perform unnecessary hashing
		if (empty($trigger_info['data']['events']) || empty($trigger_info['data']['actions']) || empty($trigger_info['active'])) {
			$trigger_info['save_hash'] = FALSE;
		}

		$tm =& $this->getTriggerManager();
		if (is_null($tm)) return FALSE;

		// use trigger manager to save
		if ($id = $tm->_saveTrigger($trigger_info)) {
			$this->id = implode(':', Array($this->tmid, $id));

			$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetUpdate', Array('short_name'));

			return TRUE;
		}
		return FALSE;

	}//end saveAttributes()


}//end class

?>
