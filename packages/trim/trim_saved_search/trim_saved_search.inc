<?php

/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: trim_saved_search.inc,v 1.54 2013/10/13 23:44:47 ewang Exp $
*
*/


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
* @author  Edison Wang <ewang@squiz.com.au>
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.54 $
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
			$ordered_result	= Array();	// A new result array with the new order if the order has been changed.

			// Now we have cache. Lets get the records
			$connection	= $this->getConnection();
			if (!empty($connection['wsdl'])) {
				$new_object_list	= $this->getTRIMRecords($connection);
			} else {
				// If we dont have TRIM connection, we are done here
				return FALSE;
			}

			// If no results are returned, it might indictate a serious trim problem. So don't go ahead and delete all local files
			// Unless we are supposed to (bug #6802)
			if(!$this->attr('accept_empty_response') && empty($new_object_list)) {
			    return FALSE;
			}
			
			$updated = FALSE;

			$result_uris		= $this->resultUris($result);
			$new_result_uris	= $this->resultUris($new_object_list);

			// Delete, Update, Integrity, New

			// Delete

			$uris_to_delete	= array_diff($result_uris, $new_result_uris);
			foreach ($result as $index	=> $record_info) {
				if (isset($record_info['Uri']) && in_array($record_info['Uri'], $uris_to_delete)) {
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
						$old_version = array_get_index($record_info, 'recLastActionDate', $record_info['Version']);
						$new_version = array_get_index($new_record_info, 'recLastActionDate', $new_record_info['Version']);
						// If Forced, dont even care about version number, update anyway.
						if ($old_version !== $new_version || $forced) {
							// UPDATE Check
							if ($this->attr('download_files')) {
								$this->storeDocumentList($connection, $new_record_info);
							}//end if
							$ordered_result[$new_index]	= $new_record_info;
						} else {
							// If this is the same version
							$ordered_result[$new_index]	= $new_record_info;

							// INTEGRITY Check
							if ($this->attr('download_files') && isset($new_record_info['recDocumentType']) && $new_record_info['recDocumentType'] != 'Record has no document attached.') {
								// We have a file here, have to check whether it is still in the system
								$final_file_path	= $this->data_path.'/'.$new_record_info['Uri'];
								$files_stored		= glob($final_file_path.'/*');
	
								// If there is no file in there
								if (empty($files_stored)) {
									$this->storeDocumentList($connection, $new_record_info);
									$ordered_result[$new_index]	= $new_record_info;
								} else if (!isset($record_info['download_url'])) {

									$ordered_result[$new_index]['document_url']		= isset($new_record_info['recNumber']) ? './?a='.$this->id.'?RecordNumber='.urlencode($new_record_info['recNumber'])  : './?a='.$this->id.'?Uri='.$new_record_info['Uri'];
								}//end else if
							}//end if

						}//end else
						$updated	= TRUE;
					}//end if
				}//end foreach
			}//end foreach

			$ordered_result_uris		= $this->resultUris($result);

			// New
			if (count($new_object_list) > count($ordered_result)) {
				$uris_to_add	= array_diff($new_result_uris, $ordered_result_uris);
				foreach ($new_object_list as $index	=> $new_record_info) {
					if (in_array($new_record_info['Uri'], $uris_to_add)) {
						// We are taking it out again so we can modify it by reference
						if ($this->attr('download_files')) {
							$this->storeDocumentList($connection, $new_record_info);
						}//end if
						$ordered_result[$index]	=	$new_record_info;
						$updated = TRUE;
					}//end if
				}//end foreach
			}//end if

			// We would have to sort the keys of the array first so array_values would not sort by pointer instead of index magnitude
			ksort($ordered_result);

			// Since we would have holes in our array keys as some entries is deleted, we need to reset the keys back to 0 onward.
			// Data source expect the key values to be correctly sorted.
			$result	= array_values($ordered_result);
			if ($updated) {
				if (!empty($result) || $this->attr('accept_empty_response')) {
				    $this->cacheResult($result, $hash);
				    return TRUE;
				}
			}//end if
			return FALSE;
		} else {
			// Not cached at all, so lets send a request
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
			$result	= $object_list;
			if (!empty($result)) {
			    $this->cacheResult($result, $hash);
			    return TRUE;
			}//end if

			return FALSE;
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
		require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';

		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);

		// Check if the URI is valid and has file attached
		$result = $this->getResultSet();

		$valid_uri = FALSE;
		foreach($result as $record) {
			if (isset($record['Uri']) && $record['Uri'] == $uri && !empty($record['document_url'])) {
				$valid_uri = TRUE;
				break;
			}
		}
		if (!$valid_uri) {
			// Either this URI is not valid, or no file is attached to this URI
			return FALSE;
		}

		$dir_path	= $this->data_path.'/'.$uri;

		$dir_path = realpath($dir_path);
		if(strpos(dirname($dir_path), $this->data_path) !== 0)
		    return FALSE;

		$path_parts	= Array();
		if (file_exists($dir_path)) {
			if(defined("RecursiveDirectoryIterator::SKIP_DOTS"))
			{
			    $dir	= new RecursiveIteratorIterator(new RecursiveDirectoryIterator($dir_path, RecursiveDirectoryIterator::SKIP_DOTS));
			}
			else {
			    $dir	= new RecursiveIteratorIterator(new RecursiveDirectoryIterator($dir_path));
			}

			if (empty($dir)) return FALSE;

			// Get the latest modified file
			$pre_mtime = 0;
			foreach ($dir as $file) {
				$mtime = $file->getMTime();
				if($mtime > $pre_mtime) {
					$file_path	= $file->getPathname();
					$path_parts	= pathinfo($file_path);
					$pre_mtime = $mtime;
				}
			}//end foreach
		} else {
			return FALSE;
		}//end else

		if (empty($path_parts)) {
			return FALSE;
		}//end if

		$ext	= strtolower($path_parts['extension']);
		$type	= (empty($standards_lists_mime_types[$ext])) ? 'text/plain' : $standards_lists_mime_types[$ext];

		// Fix for IE caching
		header("Pragma: public");
		header("Expires: 0");
		header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
		header("Cache-Control: private",false);

		// We'll be outputting a file
		header('Content-Type: '.$type);

		// Set the name of the file
		header('Content-Disposition: attachment; filename="'.$path_parts['basename'].'";');
		header('Content-Length: '.filesize($file_path));

        // Get the source file
        readfile($file_path);

		return TRUE;
		@ob_flush();

	}//end getAttachment()


	/**
	* This function return all the TRIM records based on the preconfigured settings
	*
	* @return array
	* @access public
	*/
	public function getTRIMRecords($connection)
	{
		// Set a socket timeout limit
		$default_socket_timeout = ini_get('default_socket_timeout');
		ini_set('default_socket_timeout', $this->attr('sync_time_limit'));

		$containers = Array();

		$string_search_clause				= $this->getDynamicVariableValue('search_clauses');
		$parent_container_record_number	= $this->getDynamicVariableValue('search_parent_container');
		$classification_title = $this->attr('classification');
		$classification_include_sub = $this->attr('classification_sub');

		// Fetch information
		$record_properties	= $this->getDynamicVariableValue('result_fields');
		$search_limit		= $this->getDynamicVariableValue('limit');

		if(array_key_exists('recContainerTitle', $record_properties) && !array_key_exists('recContainer', $record_properties)) {
			$record_properties['recContainer'] = 'Container';
		}

		// Get classification and sub-classification structure
		$classifications = Array();
		if(!empty($classification_title)) {
			$classification_uris = TRIM_Common::getURIsFromClassification($connection, trim($classification_title));

			foreach ($classification_uris as $classification) {
				// filter out inrelevent classifications
				if($classification_include_sub) {
					if (!preg_match ('/^'.$classification_title.'( - )?/',$classification['plnTitle'])) continue;
				}
				else {
					if ($classification['plnTitle'] != $classification_title) continue;
				}
				$classifications[$classification['Uri']] = $classification['plnTitle'];
			}
			// make sure classification is in the returned results
			$record_properties['recClassification'] = 'Classification';
		}

		// Get the "last action" date for the trim records, as since TRIM 7.1 we can't rely on "last updated" ("version")
		// anymore to check if the a record needs to updated from the TRIM (bug #6229)
		$record_properties['recLastActionDate'] = 'Last Action Date';

		// Search clauses will be directly inserted to xml request
		// Because TRIM convertToXml will mess up the order
		$search_clauses_xml = TRIM_Common::constructRecordSearchClauseGroupedXml($string_search_clause, $classifications);

		$search_clauses = Array();
		if (!empty($parent_container_record_number)) {
			$parent_container_uri	= TRIM_Common::getURIFromRecordNumber($connection, $parent_container_record_number);
			$search_clauses['RecordContainerAltClause']	= TRIM_Common::constructRecordContainerAltClause($parent_container_uri);
		}//end if

		// if no search clause is provided, do not send request, otherwise TRIM 7.3 will be angry
		if(empty($search_clauses) && empty($search_clauses_xml)) return Array();


		$record_search_options	= $this->getRecordSearchOptions();

		$operations = Array (
						'Fetch'						=> TRIM_Common::constructFetch($record_properties, $search_limit),
						'RecordSearch'				=> TRIM_Common::constructRecordSearch($search_clauses, $record_search_options),
					  );

		$file_path = NULL;

		$object_list = TRIM_Common::executeRequest($connection, $operations, 'Fetch', $search_clauses_xml);

		// inserting classification grouping keywords
		if(!empty($classification_title)) {
			foreach ($object_list as $index => $record_info) {
				if (!isset($classifications[$record_info['recClassification']])) continue;
				$record_classification = $classifications[$record_info['recClassification']];
				$object_list[$index]['classification_title'] = $record_classification;
				$object_list[$index]['classification_title_1'] = $classification_title;

				if ($record_classification !== $classification_title) {
					$record_classification = preg_replace('/^'.$classification_title.' - /', '', $record_classification, 1);
					$record_classification_level = explode (' - ', $record_classification);
					$i = 2;
					foreach ($record_classification_level as $level_title) {
						$object_list[$index]['classification_title_'.$i] = $level_title;
						$i++;
					}
				}
			}
		}

		$record_type_uris = Array();
		foreach ($object_list as $index => $record_info) {
			foreach ($record_info as $key => $value) {
				// Convert GMT Datetime to Local Time
				if($this->attr('convert_timezone') && preg_match('/ GMT$/', $value)) {
					$local_datetime = date('D, d M Y H:i:s', strtotime($value));
					$object_list[$index][$key] = empty($local_datetime) ? $value : $local_datetime;
				}

				// Make sure the field name is matrix keyword friendly
				if(preg_match('/[^a-zA-Z0-9_-]/', $key)) {
					$new_key = preg_replace('/[^a-zA-Z0-9_-]+/', '', $key);
					$new_value = $object_list[$index][$key];
					unset($object_list[$index][$key]);
					$object_list[$index][$new_key] = $new_value;
				}

				// Prapare to convert Record Type uri field to readable Record Type name
				if($key === 'recRecordType') {
					$record_type_uris[] = $value;
				}
			}
		}

		// Convert Record Type from uri to name
		if(!empty($record_type_uris)) {
			//if we dont already know all record types, we have to send extra queries to get their names
			$retrieved_info = $this->attr('retrieved_fields');
			if(isset($retrieved_info['record_types'])) {
				$record_names_data = $retrieved_info['record_types'];
			}
			else {
				$record_names_data = TRIM_Common::getRecordTypeNamesFromUris($connection, $record_type_uris);
			}

			foreach ($object_list as $index => $record_info) {
				foreach ($record_info as $key => $value) {
					if($key === 'recRecordType') {
						$new_value = isset($record_names_data[$value]) ? $record_names_data[$value] : $value;
						$object_list[$index][$key] = $new_value;
					}
				}
			}
		}

		if(array_key_exists('recContainerTitle', $record_properties)) {
			$record_properties = Array('recTitle' => 1 );
			foreach ($object_list as $index => $record_info) {
				if($record_info['recContainer']!='') {
					if(!array_key_exists($record_info['recContainer'], $containers)) {
						$results = TRIM_Common::getRecordPropertiesFromUri($connection, $record_info['recContainer'], $record_properties);
						foreach($results as $r) {
							$containers[$record_info['recContainer']] = $r['recTitle'];
						}
					}
					$object_list[$index]['recContainerTitle'] = $containers[$record_info['recContainer']];
				} else {
					$object_list[$index]['recContainerTitle'] = '';
				}
			}
		}

		// replace all locations uris with its full title
		$trim6_locations = Array('recCreatorLoc',  'recOwnerLoc', 'recAuthorLoc', 'recClientLoc', 'recCurrentLoc',  'recHomeLoc',  'recOtherLoc',  'recPrimaryContactLoc',  'recRepresentLoc');
		$trim7_locations = Array( 'recCreator',  'recOwnerLocation',    'recAuthor',  'recClient',   'recAssignee',  'recHomeLocation', 'recOtherContact',  'recPrimaryContact',  'recRepresentative');
		$location_fields = array_merge($trim6_locations, $trim7_locations);
		$uris = Array();
		$object_to_fix = Array();
		foreach ($object_list as $index => $record_info) {
		    foreach ($record_info as $name => $value) {
			if( in_array($name, $location_fields) && !empty($value)) {
			    $uris[] = $value;
			    $object_to_fix[$index][$name] = $value;
			}
		    }
		}
		// now do the replacement
		if(!empty($uris)) {
		    $location_titles = TRIM_Common::getLocationTitleFromUris($connection, $uris);
		    foreach ($object_to_fix as $index => $name_value) {
			foreach ($name_value as $name => $value) {
			    $title = $location_titles[$value];
			    $object_list[$index][$name] = $title;
			}
		    }
		}

		ini_set('default_socket_timeout', $default_socket_timeout);
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
											'trace'		=> TRUE,
											'exceptions'	=> TRUE,
										  );
		// add proxy
		if(method_exists($connector, 'getProxy')) {
			$proxy = $connector->getProxy();
			$connection['authentication'] = array_merge($connection['authentication'], $proxy);
		}

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
		$record_search_options							= Array();
		$record_search_options['limit']					= $this->getDynamicVariableValue('limit');
		$record_search_options['file_types']			= $this->getDynamicVariableValue('fileTypes');
		$record_search_options['sort_1']				= $this->getDynamicVariableValue('search_sort_1');
		$record_search_options['sort_2']				= $this->getDynamicVariableValue('search_sort_2');
		$record_search_options['sort_3']				= $this->getDynamicVariableValue('search_sort_3');
		$record_search_options['sort_1_descending']		= $this->getDynamicVariableValue('sort_1_descending');
		$record_search_options['sort_2_descending']		= $this->getDynamicVariableValue('sort_2_descending');
		$record_search_options['sort_3_descending']		= $this->getDynamicVariableValue('sort_3_descending');
		$record_search_options['record_type_filter']    = is_array($this->getDynamicVariableValue('record_type_filter')) ? array_keys ($this->getDynamicVariableValue('record_type_filter')) : '';

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
		    	// Set a socket timeout limit
			$default_socket_timeout = ini_get('default_socket_timeout');
			ini_set('default_socket_timeout', $this->attr('download_time_limit'));
			$record_info['document_url']	= '';

			if (isset($record_info['recDocumentType']) && $record_info['recDocumentType']	!= 'Record has no document attached.') {
				// We got a file here.
				$operations = Array (
								'Download'					=> TRIM_Common::constructDownload($this->attr('download_limit')),
								'ShortcutRecordUri'			=> TRIM_Common::constructShortcutRecordUri($record_info['Uri']),
							  );

				$final_file_path	= $this->data_path.'/'.$record_info['Uri'];
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				create_directory($final_file_path);

				$filename = '';
				$filename_field = $this->attr('filename_field');
				if (!empty($filename_field)) {
					$filename = array_get_index($record_info, $filename_field, '');
				}

				$res = TRIM_Common::executeRequest($connection, $operations, 'Download', '',  $filename, $final_file_path);

				// use RecordNumber as id of downloaded file because it makes sense to user. Uri is only internal to developer
				$record_info['document_url'] = isset($record_info['recNumber']) ? './?a='.$this->id.'?RecordNumber='.urlencode($record_info['recNumber'])  : './?a='.$this->id.'?Uri='.$record_info['Uri'];

			}//end if

			ini_set('default_socket_timeout', $default_socket_timeout);
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
	* Function printFrontend
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{

		if ($this->readAccess()) {
			// don't let browser cache it
		    	header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
			header('Pragma: no-cache');
			header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');

			$connection	= $this->getConnection();
			if (isset($_GET['Uri']) && !empty($_GET['Uri'])) {
				$download_result	= $this->getAttachedFile(($_GET['Uri']));
				if (!$download_result) {
					trigger_error('Unable to retrieve file', E_USER_WARNING);
				}//end if
			}
			if (isset($_GET['RecordNumber']) && !empty($_GET['RecordNumber'])) {
				$results = $this->getResultSet();
				$uri = NULL;
				foreach ($results as $result) {
				    if(isset($result['recNumber']) && $result['recNumber'] === $_GET['RecordNumber'] && isset($result['Uri'])) {
					$uri = $result['Uri'];
					break;
				    }
				}
				$download_result	= $this->getAttachedFile($uri);
				if (!$download_result) {
					trigger_error('Unable to retrieve file', E_USER_WARNING);
				}//end if
			}
			else if (isset($_GET['check_update']) && !empty($_GET['check_update'])) {
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
									$old_version = array_get_index($record_info, 'recLastActionDate', $record_info['Version']);
									$new_version = array_get_index($new_record_info, 'recLastActionDate', $new_record_info['Version']);
									if ($old_version !== $new_version) {
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
			} else if (isset($_GET['force_update']) && !empty($_GET['force_update'])) {
				if ($this->adminAccess()) {
					if ($this->updateCachedResult(TRUE)) {
						echo '1';
					} else {
						echo '0';
					}//end if
				}//end if
			}//end else if
		}//end if
		else {
		    	$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}
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
		return parent::setAttrValue($name, $value);
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


	/**
	* Returns retrieved fields info from TRIM server
	*
	* @param string		$type		what kind of fields to return? additional fields, standard fields or all
	*
	* @return array
	* @access public
	*/
	public function getRetrievedFields($type = 'all')
	{
		$fields = $this->attr('retrieved_fields');
		if(empty($fields)) return Array();

		if ($type == 'additional') {
			return $fields['additional_fields'];
		}
		else if ($type == 'standard') {
			return $fields['standard_fields'];
		}
		else {
			return (array_merge($fields['standard_fields'], $fields['additional_fields']));
		}
	}

	/**
	* Returns the type of a retrieved field
	*
	* @param string		$field		name of the retrieved field
	*
	* @return string
	* @access public
	*/
	public function getRetrievedFieldType($field)
	{
		$fields = $this->attr('retrieved_fields');
		if(empty($fields) || !isset($fields['field_types'])) return '';
		$field_types = $fields['field_types'];
		return (isset($field_types[$field]) ? $field_types[$field] : '');
	}

}//end class
?>
