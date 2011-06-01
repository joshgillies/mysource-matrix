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
* $Id: trim_saved_search.inc,v 1.9 2009/01/27 05:35:16 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_PACKAGES_PATH.'/trim/lib/trim_common.inc';

/**
* Data_Source_TRIM
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __trim__
*/
class TRIM_Saved_Search extends Data_Source
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
		$this->setAttrValue('shadow_name', '%ds__recTitle%');

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

		$result = $this->getCachedResults($hash);
		if (empty($result)) {
			try {
				// Not cached at all, so lets send a request
				$res	= Array();
				$connection	= $this->getConnection();
				$object_list	= '';
				if (!empty($connection['wsdl'])) {
					$object_list	= $this->getTRIMRecords($connection);
					// Only download files if we are asked to
					if ($this->attr('download_files')) {
						foreach ($object_list as &$record_info) {
							$this->storeDocumentList($connection, $record_info);
						}//end foreach
					}//end if
					unset($record_info);
				}//end if

				if (!isset($res[0]['error'])) {
						$result	= $object_list;
						if (!empty($result)) {
							$this->cacheResult($result, $hash);
						}//end if
				} else if (isset($res[0]['FoundCount']) && $res[0]['FoundCount'] != 0) {
					// We got some result, but there is error, lets throw an error.
				}//end else if
			} catch (Exception $sf) {
				//trigger_error($res['error'], E_USER_WARNING);
			}//end try catch
		}//end else

		return $result;

	}//end getResultSet()


	public function getCachedResults($hash_key)
	{
		$result = $this->attr('cached_result');
		if (isset($result[$hash_key])) {
			return $result[$hash_key];
		}//end if

		return Array();

	}//end cacheResult()


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

		// Check the cache
		$result = $this->getCachedResults($hash);
		if (!empty($result)) {
			// Now we have cache. Lets get the records
			$connection	= $this->getConnection();
			if (!empty($connection['wsdl'])) {
				$new_object_list	= $this->getTRIMRecords($connection);
			}//end if

			$updated = FALSE;

			$result_uris		= $this->resultUris($result);
			$new_result_uris	= $this->resultUris($new_object_list);

			// Delete, Update, Integrity, New

			// Delete

			$uris_to_delete	= array_diff($result_uris, $new_result_uris);
			foreach ($result as $index	=> $record_info) {
				if (in_array($record_info['Uri'], $uris_to_delete)) {
					// Unlink file first
					$dir_path	= $this->data_path.'/'.$record_info['Uri'];
					$files_stored		= glob($dir_path.'/*');
					foreach ($files_stored as $file_name) {
						unlink($file_name);
					}//end foreach

					// Unlink the directory
					if (is_dir($dir_path)) {
						rmdir($dir_path);
					}//end if

					// Remove it from the cache
					unset($result[$index]);
					$updated = TRUE;
				}//end if
			}//end foreach
			// We have something to delete


			// Update & Integrity
			foreach ($result as $index	=> $record_info) {
				foreach ($new_object_list as $new_index	=> $new_record_info) {
					// Comparing the same record based on the URI
					if ($record_info['Uri'] == $new_record_info['Uri']) {
						// If Forced, dont even care about version number, update anyway.
						if ($record_info['Version'] !== $new_record_info['Version'] || $forced) {
							// UPDATE Check
							$result[$index]	= $new_record_info;
							if ($this->attr('download_files')) {
								$this->storeDocumentList($connection, $new_record_info);
							}//end if
							$updated = TRUE;
						} else {
							// INTEGRITY Check
							if (isset($new_record_info['recDocumentType']) && $new_record_info['recDocumentType'] != 'Record has no document attached.') {
								// We have a file here, have to check whether it is still in the system
								$final_file_path	= $this->data_path.'/'.$new_record_info['Uri'];
								$files_stored		= glob($final_file_path.'/*');

								// If there is no file in there
								if (empty($files_stored)) {
									$this->storeDocumentList($connection, $new_record_info);
									$updated = TRUE;
								} else if (!isset($record_info['download_url'])) {
									$result[$index]['document_url']		= $this->getURL().'?Uri='.$new_record_info['Uri'];
									$updated = TRUE;
								}//end else if
							}//end if

						}//end else
					}//end if
				}//end foreach
			}//end foreach

			// New
			if (count($new_object_list) > count($result)) {
				$uris_to_add	= array_diff($new_result_uris, $result_uris);
				foreach ($new_object_list as $index	=> $new_record_info) {
					if (in_array($new_record_info['Uri'], $uris_to_add)) {
						// We are taking it out again so we can modify it by reference
						if ($this->attr('download_files')) {
							$this->storeDocumentList($connection, $new_record_info);
						}//end if
						$result[]	=	$new_record_info;
						$updated = TRUE;
					}//end if
				}//end foreach
			}//end if

			// Since we would have holes in our array keys as some entries is deleted, we need to reset the keys back to 0 onward.
			// Data source expect the key values to be correctly sorted.
			$result	= array_values($result);
			if ($updated) {
				if (!empty($result)) {
					$this->cacheResult($result, $hash);
					return TRUE;
				}//end if
			}//end if
			return FALSE;
		} else {
			$this->getResultSet();
			return TRUE;
		}//end else

	}//end updateCachedResult()


	/**
	 * This function return an array of Uris
	 */
	public function resultUris($result)
	{
		$new_result	= Array();
		foreach ($result as $index => $record_info) {
			$new_result[$record_info['Uri']] = $record_info;
		}//end foreach

		return array_keys($new_result);

	}//end convertResultToAssoc


	/**
	* This function get attached file(s) for a particular URI
	*
	* @return void
	* @access public
	*/
	public function getAttachedFile($uri)
	{
		$dir_path	= $this->data_path.'/'.$uri;
		if (file_exists($dir_path)) {
			$dir	= new RecursiveIteratorIterator(new RecursiveDirectoryIterator($dir_path));
			if (empty($dir)) return FALSE;
			foreach ($dir as $file) {
				$file_path	= $file->getPathname();
				$path_parts	= pathinfo($file_path);
			}//end foreach
		} else {
			return FALSE;
		}

		// Fix for IE caching
		header("Pragma: public");
		header("Expires: 0");
		header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
		header("Cache-Control: private",false);

		// We'll be outputting a file
		header('Content-type: application/'.strtolower($path_parts['extension']));

		// Set the name of the file
		header('Content-Disposition: attachment; filename="'.$path_parts['basename'].'";');
		header('Content-Length: '.filesize($file_path));

		// Get the source file
		readfile($file_path);

		return TRUE;

	}//end getAttachment()


	/**
	* This function return all the TRIM records based on the preconfigured settings
	*
	* @return array
	* @access public
	*/
	public function getTRIMRecords($connection)
	{
		$search_kw 						= $this->getDynamicVariableValue('search_keyword');
		$string_search_type				= $this->getDynamicVariableValue('string_search_type');
		$parent_container_record_number	= $this->getDynamicVariableValue('search_parent_container');

		// Fetch information
		$record_properties	= $this->getDynamicVariableValue('fields');
		$search_limit		= $this->getDynamicVariableValue('limit');

		$search_clauses = Array();
		if (!empty($search_kw) && !empty($string_search_type)) {
			$search_type	= $this->getDynamicVariableValue('string_search_type');
			$search_keyword	= $this->getDynamicVariableValue('search_keyword');
			$search_clauses['RecordStringSearchClause']	= TRIM_Common::constructRecordStringSearchClause($search_type, $search_keyword);
		}//end if


		if (!empty($parent_container_record_number)) {
			$parent_container_uri	= TRIM_Common::getURIFromRecordNumber($connection, $parent_container_record_number);
			$search_clauses['RecordContainerAltClause']	= TRIM_Common::constructRecordContainerAltClause($parent_container_uri);
		}//end if

		$record_search_options	= $this->getRecordSearchOptions();

		$operations = Array (
						'Fetch'						=> TRIM_Common::constructFetch($record_properties, $search_limit),
						'RecordSearch'				=> TRIM_Common::constructRecordSearch($search_clauses, $record_search_options),
					  );

		$object_list = TRIM_Common::executeRequest($connection, $operations, 'Fetch');

		return $object_list;

	}//end getTRIMRecords();


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
		$search_kw 						= $this->getDynamicVariableValue('search_keyword');
		$string_search_type				= $this->getDynamicVariableValue('string_search_type');
		$parent_container_record_number	= $this->getDynamicVariableValue('search_parent_container');

		$unhashed_key = $search_kw
						.$parent_container_record_number
						.$this->getDynamicVariableValue('limit')
						.$string_search_type
						.implode('-',$this->getDynamicVariableValue('fields'))
						.$this->getDynamicVariableValue('fileTypes');

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
		$connector_link					= $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'trim_connector', FALSE, 'trim_connector');
		if (!empty($connector_link)) {
			$connector					= $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_link['minorid']);
		} else {
			$connector					= $this;
		}//end else

		$connection						= Array();
		$connection['wsdl']				= $connector->attr('wsdl');
		$connection['authentication']	= Array (
											'login'		=> $connector->attr('username'),
											'password'	=> $connector->attr('password'),
											'trace'		=> '1',
											'exception'	=> '1',
										  );

		return $connection;

	}//end getConnection()


	/**
	* This function populate the search options.
	*
	* @return array
	* @access public
	*/
	function getRecordSearchOptions()
	{
		$record_search_options						= Array();
		$record_search_options['limit']				= $this->getDynamicVariableValue('limit');
		$record_search_options['file_types']		= $this->getDynamicVariableValue('fileTypes');
		$record_search_options['sort_1']			= $this->getDynamicVariableValue('search_sort_1');
		$record_search_options['sort_2']			= $this->getDynamicVariableValue('search_sort_2');
		$record_search_options['sort_3']			= $this->getDynamicVariableValue('search_sort_3');
		$record_search_options['sort_1_descending']	= $this->getDynamicVariableValue('sort_1_descending');
		$record_search_options['sort_2_descending']	= $this->getDynamicVariableValue('sort_2_descending');
		$record_search_options['sort_3_descending']	= $this->getDynamicVariableValue('sort_3_descending');

		return $record_search_options;

	}//end getRecordSearchOptions()


	/**
	* This function updates all the documents.
	*
	* @return void
	* @access public
	*/
	function storeDocumentList($connection, &$record_info)
	{
		if (isset($record_info['Uri']) && $record_info['Uri'] != 0 && (!isset($record_info['error']))) {
			$record_info['document_url']	= '';
			if ($record_info['recDocumentType']	!= 'Record has no document attached.') {
				// We got a file here.
				$operations = Array (
								'Download'					=> TRIM_Common::constructDownload(),
								'ShortcutRecordUri'			=> TRIM_Common::constructShortcutRecordUri($record_info['Uri']),
							  );

				$hash_key			= $GLOBALS['SQ_SYSTEM']->user->attr('username').time();
				$final_file_path	= $this->data_path.'/'.$record_info['Uri'];
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				create_directory($final_file_path);

				$res = TRIM_Common::executeRequest($connection, $operations, 'Download', $hash_key, $final_file_path);
				$record_info['document_url']	= $this->getURL().'?Uri='.$record_info['Uri'];
			}//end if
		}//end if

	}//end storeDocumentList()


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
	* This function is used to clean the dynamic vars list when the data source search option is changed
	*
	* @return void
	* @access public
	*/
	public function cleanupDynamicVars(&$dynamic_vars)
	{
		$remove_keys	= Array ('search_keyword', 'string_search_type', 'StartTime', 'EndTime', 'dateRangeSearchType', 'StartNum', 'EndNum', 'UseExpandedNumber');

		foreach ($remove_keys as $key) {
			if (isset($dynamic_vars[$key])) {
				unset($dynamic_vars[$key]);
			}//end else
		}//end foreach

	}//end cleanupDynamicVars()


	/**
	* Function printFrontend
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		if ($this->readAccess()) {
			$connection	= $this->getConnection();
			if (isset($_GET['Uri']) && !empty($_GET['Uri'])) {
				$this->getAttachedFile(($_GET['Uri']));
			} else if (isset($_GET['check_update']) && !empty($_GET['check_update'])) {
				if ($this->adminAccess()) {
					echo 'var numUpdate = false;';
					$new_object_list	= $this->getTRIMRecords($connection);

					$hash	= $this->getHashKey();
					$result = $this->getCachedResults($hash);

					if (!empty($result)) {
						// We have some cache. Lets check them one by one.
						$i = 0;
						foreach ($result as $index	=> $record_info) {
							foreach ($new_object_list as $new_index	=> $new_record_info) {
								// Comparing the same record based on the URI
								if ($record_info['Uri'] == $new_record_info['Uri']) {
									if ($record_info['Version'] !== $new_record_info['Version']) {
										$i++;
									}//end if
								}//end if
							}//end foreach
						}//end foreach

						// If we have records to add or delete, have to account for them as well
						$final_count	= $i+abs(count($result) - count($new_object_list));
						echo 'var numUpdate = '.$final_count.';';
					} else {
						// We have no cache. Just return the count in object_list
						echo 'var numUpdate = '.count($new_object_list).';';
					}//end else

				}//end if
			} else if (isset($_GET['synch_now']) && !empty($_GET['synch_now'])) {
				if ($this->adminAccess()) {
					if ($this->updateCachedResult()) {
						echo '1';
					} else {
						echo '0';
					}//end if
				}//end if
			}//end else if
		}//end if
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
		if ($name == 'download_files') {
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
