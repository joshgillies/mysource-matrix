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
* $Id: bodycopy.inc,v 1.53 2012/08/30 01:09:05 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Bodycopy
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.53 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy extends Asset
{

	/**
	* An array of keywords and their replacements
	* @var array
	*/
	public $keyword_replacements = Array();


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	public function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Create this asset
	*
	* @param array	&$link		information used to create the initial link
	* @param array	$bc_data	details for this bodycopy
	*							['attributes']['identifier'] => id for the default div container
	*							['content'] => html content for the default div container
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link, $bc_data=Array())
	{
		// store this data for later use during the create process in _createAdditional()
		$this->_tmp['bodycopy_create_data'] = $bc_data;

		return parent::create($link);

	}//end create()


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Bodycopies ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array $link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Bodycopies create a DIV when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		// grab the bodycopy data that we stored in the create() function
		$bc_data = $this->_tmp['bodycopy_create_data'];
		unset($this->_tmp['bodycopy_create_data']);

		if (!parent::_createAdditional($link)) return FALSE;

		// create a DIV
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy_div');
		$bodycopy_div = new Bodycopy_DIV();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'is_dependant' => 1);
		$bodycopy_div->setAttrValue('name', 'Content DIV');
		$create_success = $bodycopy_div->create($copy_link, $bc_data);
		return $create_success;

	}//end _createAdditional()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a bodycopy container then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Bodycopy_Container) && $dependant != '1') {
			$dependant = '1';
			$link_type = SQ_LINK_TYPE_2;
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_2 => Array(
									'bodycopy_container' => Array(
																'card'		=> 'M',
																'exclusive'	=> FALSE,
															),
								  ),
			   );

	}//end _getAllowedLinks()


	/**
	* Provides extra checking and disable invalid status change on errors 
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parents=TRUE)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directories exists
		if (!create_directory($this->data_path)) {
			trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
			return FALSE;
		}

		switch ($new_status) {
			case SQ_STATUS_EDITING:
				// Going to safe edit
				if ($this->status == SQ_STATUS_LIVE) {
					$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
					file_put_contents($this->data_path.'/.safe_edit_before', serialize($containers));
					file_put_contents($this->data_path.'/.safe_edit_after', serialize($containers));
				}//end if
			break;
			case SQ_STATUS_LIVE:
				if ($this->status == SQ_STATUS_EDITING) {
					// Cancel safe edit
					$before_safe_edit = Array();
					$after_safe_edit = Array();
					$before_safe_edit_string = file_to_string($this->data_path.'/.safe_edit_before');
					$after_safe_edit_string = file_to_string($this->data_path.'/.safe_edit_after');

					if (!empty($before_safe_edit_string)) {
						$before_safe_edit = @unserialize($before_safe_edit_string);
					}//end if
					if (!empty($after_safe_edit_string)) {
						$after_safe_edit = @unserialize($after_safe_edit_string);
					}//end if

					// Remove DIVs if they did not exist before going back to Safe Edit
					if (!empty($after_safe_edit)) {
						foreach ($after_safe_edit as $active_link) {
							if (!$this->_compareLink($active_link, $before_safe_edit)) {
								// The link didn't exist, move to trash
								if (isset($active_link['linkid'])) {
									// Drop to a run level to allow the links to be shuffled out without a fuss
									$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
									$this->deleteLink($active_link['linkid']);	
									$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
								}//end if
							}//end if
						}//end foreach
					}//end if

					// Coming out of safe edit remove safe edit files
					$this->_cleanupSafeEdit();
				}//end if

				if ($this->status == SQ_STATUS_EDITING_APPROVED) {
					// Changes approved
					$this->_cleanupSafeEdit();
				}//end if
			break;
		}//end switch

		return parent::processStatusChange($new_status, $update_parents);

	}//end processStatusChange()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object	&$o	Backend_Outputter instance to use for painting
	*
	* @return void
	* @access public
	*/
	public function paintBackend(Backend_Outputter $o)
	{	
		if (!isset($_REQUEST['asset_ei_screen'])) {
			return parent::paintBackend($o);
		}
		switch ($_REQUEST['asset_ei_screen']) {
			case 'show_popup' :
				$include_file = trim($_REQUEST['popup_file']);
				$include_dir = dirname(__FILE__).'/popups/';
				$dir_listing = scandir($include_dir);

				if (in_array($include_file, $dir_listing)) {
					require_once($include_dir.$include_file);
				} 
				exit();
			default :
				return parent::paintBackend($o);

		}//end switch

	}//end paintBackend()


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		if (!isset($_REQUEST['asset_ei_screen'])) {
			return parent::printFrontend();
		}
		switch ($_REQUEST['asset_ei_screen']) {
			case 'show_popup' :
				$include_file = trim($_REQUEST['popup_file']);
				$include_dir = dirname(__FILE__).'/popups/';
				$dir_listing = scandir($include_dir);

				if (in_array($include_file, $dir_listing)) {
					require_once($include_dir.$include_file);
				} 
				exit();
			default :
				return parent::printFrontend();

		}//end switch

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return boolean
	* @access public
	*/
	public function printBody()
	{
		// get all the containers linked to this bodycopy
		$containers =& $this->getContainers();
		
		// if conditional display of div is enabled
		if(isset($GLOBALS['SQ_CONDITION']['conditional_div'])) {
		    foreach (array_keys($containers) as $i) {
			$container =& $containers[$i];
			if(isset($GLOBALS['SQ_CONDITION']['conditional_div'][$container->id]) && !$GLOBALS['SQ_CONDITION']['conditional_div'][$container->id]) {
			    // do not display div that is evaled false
			    unset($containers[$i]);
			}
		    }
		}

		// paint each container
		foreach (array_keys($containers) as $i) {
			$container =& $containers[$i];
			$container->printBody($this->keyword_replacements);
		}

		return TRUE;

	}//end printBody()


	/**
	* Get the name or short name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to use to return the name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* To get all of the containers linked to this bodycopy
	*
	* Returns an array of Bodycopy_Container objects
	*
	* @return array
	* @access public
	*/
	public function &getContainers()
	{
		if (SQ_IN_BACKEND || SQ_IN_LIMBO || SQ_PHP_CLI || !isset($this->_tmp['containers'])) {
			$this->_tmp['containers'] = Array();
			$container_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
			foreach ($container_links as $container_data) {
				$this->_tmp['containers'][] = $GLOBALS['SQ_SYSTEM']->am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			}
		}
		return $this->_tmp['containers'];

	}//end getContainers()


	/**
	* Determine if there is no content in this bodycopy
	*
	* @return boolean
	* @access public
	*/
	public function isEmpty()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE);
		foreach ($containers as $container_data) {
			$container = $am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			if (!$container->isEmpty()) return FALSE;
		}
		return TRUE;

	}//end isEmpty()


	/**
	* To get the keywords that exist in the content of the bodycopy.
	*
	* @return array
	* @access public
	*/
	public function getKeywords()
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$content = $this->getRawBodycopyContent();
		$keywords = retrieve_keywords_replacements($content);
		return $keywords;

	}//end getKeywords()


	/**
	* Returns the raw content of this bodycopy
	*
	* @return string
	* @access public
	*/
	public function getContent()
	{
		return $this->getRawBodycopyContent();

	}//end getContent()


	/**
	* Get the raw content of this bodycopy
	*
	* @return string
	* @access public
	*/
	public function getRawBodycopyContent()
	{
		$containers =& $this->getContainers();
		$contents = '';

		ob_start();
			foreach (array_keys($containers) as $i) {
				$container =& $containers[$i]; // need this by reference
				$edit = $container->getEditFns();
				$edit->paint($container, FALSE, TRUE);
			}
			$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getRawBodycopyContent()


	/**
	* Sets the Array of keywords with their replacements
	*
	* @param array	$replacements	an array with the keyword as the key and the replacement as the value
	*
	* @return boolean
	* @access public
	*/
	public function setKeywordReplacements($replacements=Array())
	{
		if (!is_array($replacements)) return FALSE;
		if (empty($replacements)) return FALSE;

		$this->keyword_replacements = $replacements;
		return TRUE;

	}//end setKeywordReplacements()


	/**
	* Compares the before and after state to check for div/table removal 
	*
	* @param array		$current_link	The info about the current link
	* @param array		$before_links	An array of all the before links
	*
	* @return boolean
	* @access private
	*/
	function _compareLink($current_link, $before_links)
	{
		if (!isset($current_link['minorid'])) {
			// Not enough information to proceed
			return FALSE;
		}//end if

		foreach ($before_links as $before_link) {
			if (isset($before_link['minorid'])) {
				if ($before_link['minorid'] == $current_link['minorid']) {
					// It exists!
					return TRUE;
				}//end if
			}//end if
		}//end foreach

		// Didn't find one, report that...
		return FALSE;
	
	}//end _compareLink()


	/**
	* Cleaning up files after coming out of Safe Edit 
	*
	* @return void
	* @access private
	*/
	function _cleanupSafeEdit()
	{
		if (file_exists($this->data_path.'/.safe_edit_after')) {
			unlink($this->data_path.'/.safe_edit_after');
		}//end if
		if (file_exists($this->data_path.'/.safe_edit_before')) {
			unlink($this->data_path.'/.safe_edit_before');
		}//end if
	
	}//end _cleanupSafeEdit()


}//end class

?>
