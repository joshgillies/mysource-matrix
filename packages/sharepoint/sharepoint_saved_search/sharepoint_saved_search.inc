<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: sharepoint_saved_search.inc,v 1.3 2009/01/09 04:07:04 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_PACKAGES_PATH.'/sharepoint/lib/sharepoint_common.inc';

/**
* Sharepoint_Saved_Search
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __sharepoint__
*/
class Sharepoint_Saved_Search extends Data_Source
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

		$this->_ser_attrs = TRUE;
		$this->search_clauses = NULL;

	}//end constructor


    /**
    * Perform any additional processing required during the creation of this asset
    *
    * Standard pages create a bodycopy asset when they are created
    *
    * @param array  &$link  information used to create the initial link
    *
    * @return boolean
    * @access private
    */
    protected function _createAdditional(Array &$link, $args=Array())
    {
    	if (!parent::_createAdditional($link)) return FALSE;
		$this->setAttrValue('shadow_name', '%ds__ows_FileLeafRef%');

		return TRUE;

    }//end _createAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	public function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()


	/**
	* Returns an array of the result of query execution
	*
	* @return array
	* @access public
	*/
	public function &getResultSet()
	{
		$hash	= $this->getHashKey();

		$data_list	= $this->attr('data_list');
		if (!empty($data_list) && $data_list != 'None') {
			$result = $this->getCachedResults($hash);
			if (empty($result)) {
				try {
					$connection	= $this->getConnection();

					$all_lists		= Array();
					$lists_fields	= Array();
					$list_names		= Array();

					$valid = Sharepoint_Common::test_sharepoint_connection($connection);
					//$connection['url']	= Sharepoint_Common::getWSDLLocation('lists', $connection['url']);
					if ($valid) {
						$all_lists	= Sharepoint_Common::getListCollections($connection);
						$sp_lists	= Sharepoint_Common::getListNames($connection, $all_lists);
						$list_names	+= $sp_lists;
					}//end if

					$search_crits	= $this->attr('search_options');
					$search_logic	= $this->attr('search_logic');
					$row_limit		= $this->attr('row_limit');
					$search_crit	= isset($search_crits[$data_list]) ? $search_crits[$data_list] : Array();

					$result = Sharepoint_Common::getListItems($connection, $data_list, $search_crit, $search_logic, $row_limit);
					if (!empty($result)) sort($result);
					$this->cacheResult($result, $hash);

				} catch (SoapFault $sf) {
					$detail = '';
					if (isset($sf->detail)) $detail = $sf->detail;
					trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
				}//end try catch
			}//end else
		}//end if

		return $result;

	}//end getResultSet()


	/**
	* Return the cached result
	*
	* @param string	$hash_key	The hash key that this asset used to cache the result
	*
	* @return array
	* @access public
	*/
	public function getCachedResults($hash_key)
	{
		$result = $this->attr('cached_result');
		if (isset($result[$hash_key])) {
			return $result[$hash_key];
		}//end if

		return Array();

	}//end cacheResult()


	/**
	* Cache the result
	*
	* @param array	$result		The result from the external system
	* @param string	$hash_key	The hash key
	*
	* @return boolean
	* @access public
	*/
	public function cacheResult($result, $hash_key)
	{
		$cached_result = $this->attr('cached_result');
		$cached_result[$hash_key]	= $result;
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes');
		if ($this->setAttrValue('cached_result', $cached_result) && $this->saveAttributes()) {
			return TRUE;
		}//end if
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');

		return FALSE;

	}//end cacheResult()


	/**
	* This function update only records which has the version has been changed
	*
	* @return void
	* @access public
	*/
	public function updateCachedResult($forced=FALSE)
	{
		$hash	= $this->getHashKey();

		$result = $this->attr('cached_result');
		if (isset($result[$hash])) {
			unset($result[$hash]);
		}//end if
		$this->setAttrValue('cached_result', $result);
		$new_result	= $this->getResultSet();
		if (!empty($new_result)) {
			return TRUE;
		} else {
			return FALSE;
		}//end else

	}//end updateCachedResult()


	/**
	* This function return the hash string for a particular search settings
	*
	* @return string
	* @access string
	*/
	function getHashKey() {

		// Hash Key was used before when this asset uses the Cache Manager to manage its cache
		// Since attribute is used now, it's hard to go back and clear the cache for different configuration
		// so lets just use one cache set instead of multiple sets like before
		/*
		$hash = md5($unhashed_key);*/
		$hash = md5('resultSet');

		return $hash;

	}//end getHashKey()


	/**
	* This function return the connection details
	*
	* @return array
	* @access public
	*/
	function getConnection()
	{
		$connector_link					= $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'sharepoint_connector', FALSE, 'sharepoint_connector');
		if (!empty($connector_link)) {
			$connector					= $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['minorid']);
		} else {
			$connector					= $this;
		}//end else

		$connection						= Array();
		$connection['url']				= $connector->attr('url');
		$connection['authentication']	= Array (
											'login'		=> $connector->attr('username'),
											'password'	=> $connector->attr('password'),
											'trace'		=> '1',
											'exception'	=> '1',
										  );

		return $connection;

	}//end getConnection()


	/**
	* Get a value of a dynamic variable
	*
	* @param string	$variable_name	Name of variable
	*
	* @return string
	* @access public
	*/
	public function getDynamicVariableValue($variable_name)
	{
		$dynamic_vars = $this->attr('dynamic_vars');
		if (!isset($dynamic_vars[$variable_name])) return '';

		$parameter_map = $this->getAttribute('parameter_map');
		$value = $parameter_map->getParameterValue($variable_name);

		if (empty($value)) {
			$value = array_get_index($dynamic_vars, $variable_name, '');
		}//end if

		return $value;

	}//end getDynamicVariableValue()


	/**
	* Function printFrontend
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		// We are only doing synching in this asset, there's no need to check for update
		if (isset($_GET['synch_now']) && !empty($_GET['synch_now'])) {
			if ($this->adminAccess()) {
				if ($this->updateCachedResult()) {
					echo '1';
				} else {
					echo '0';
				}//end if
			}//end if
		}//end else if

	}//end printFrontend()


    /**
    * Queue the setting of the value of an attribute
    *
    * This function sets the value of the attribute locally, but does not database updating.
    * The change is queued awaiting the call of saveAttributes()
    *
    * @param string $name   the name of the attribute
    * @param string $value  the new value of the attribute
    *
    * @return boolean
    * @access public
    * @see saveAttributes()
    */
	public function setAttrValue($name, $value)
	{
		if ($name == 'data_list' || $name == 'row_limit' || $name == 'search_logic') {
			$res_val	= parent::setAttrValue($name, $value);
			$this->updateCachedResult(TRUE);
			return $res_val;
		} else {
			return parent::setAttrValue($name, $value);
		}//end if
	}//end setAttrValue()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object
	* @access public
	* @see Asset_Manager::getAsset()
	*/
	public function getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
		$asset	= parent::getAsset($assetid, $type_code='', $mute_errors=FALSE);
		if (isset($_REQUEST['a']) && isset($_REQUEST['a']) == $assetid) {
			// Now we are looking for shadow assets to download
			if (!is_null($asset)) {
				if (isset($asset->vars['Uri']) && !empty($asset->vars['Uri'])) {
					if ($this->getAttachedFile(($asset->vars['Uri']))) {
						exit();
					}
				}//end if
			}//end if
		}//end if

		return $asset;
	}//end getAsset()


}//end class
?>
