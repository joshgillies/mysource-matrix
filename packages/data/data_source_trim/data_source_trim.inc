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
* $Id: data_source_trim.inc,v 1.2 2008/03/18 21:50:30 hnguyen Exp $
*
*/

//check_ssv_licence();
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';

/**
* Data_Source_TRIM
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_TRIM extends Data_Source
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
		$search_kw = $this->getDynamicVariableValue('search_keyword');
		$string_search_type = $this->getDynamicVariableValue('string_search_type');

		$search_start_time	= $this->getDynamicVariableValue('search_start_time');
		$search_end_time	= $this->getDynamicVariableValue('search_end_time');

		$search_start_rec_number	= $this->getDynamicVariableValue('search_start_rec_number');
		$search_end_rec_number		= $this->getDynamicVariableValue('search_end_rec_number');

		$unhashed_key = $search_kw
						.$this->getDynamicVariableValue('limit')
						.$string_search_type
						.implode('-',$this->getDynamicVariableValue('fields'))
						.$this->getDynamicVariableValue('fileTypes');

		$hash = md5($unhashed_key);
		if (!isset($this->_tmp[$hash])) {
			// Check the system cache
			$result = $this->getCachedResultSet($hash);
			if ($result !== FALSE) {
				$this->_tmp[$hash]	= $result;
			} else {
				try {
					// Not cached at all, so lets send a request
					$res	= Array();
					$wsdl	= $this->attr('wsdl');
					if (!empty($wsdl)) {
						$operations = Array (
										'Fetch'						=> $this->constructFetch(),
										'RecordSearch'				=> $this->constructRecordSearch(),
									  );

						$res = $this->executeRequest($operations, 'Fetch');
					}//end if
					if (!isset($res[0]['error'])) {
							$this->_tmp[$hash]	= $res;
							if (!empty($this->_tmp[$hash])) {
								$this->cacheResultSet($this->_tmp[$hash], $hash);
							}//end if
					} else if (isset($res[0]['FoundCount']) && $res[0]['FoundCount'] != 0) {
						// We got some result, but there is error, lets throw an error.
					}//end else if
				} catch (Exception $sf) {
					//trigger_error($res['error'], E_USER_WARNING);
				}//end try catch
			}//end else
		}
		// Fall-through failure case

		return $this->_tmp[$hash];

	}//end getResultSet()


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
	* Construct an array represent the TRIMRequest object being sent
	*
	* @return unknown
	* @access public
	*/
	public function constructTrimRequest()
	{
		$trim_request	= Array (
							'HideVersionNumbers'		=> TRUE,
							'ProvideTimingResults'		=> FALSE,
							'ForceRealTimeCacheUpdate'	=> TRUE,
						  );

		return $trim_request;

	}//end constructTrimRequest()


	/**
	* function to execute the web service request once all the internal objects have been set up properly
	*
	* @param array 	$operations		the operations that can be perform (search, fetch, download, upload, getaccess)
	* @param string	$display_type	the major operation that need to be achieved, so the object know how to parse the returned XML.
	*
	* @return void
	* @access public
	*/
	public function executeRequest($operations=Array(), $display_type='Fetch', $hash_key='', &$final_file_path='')
	{
		$Execute		= Array (
							'req'	=> $this->constructTrimRequest(),
						  );
		$Execute['req']		= array_merge($Execute['req'], $operations);

		// Get the soapclient object.
		$authen_info	= Array (
							'login'		=> $this->attr('username'),
							'password'	=> $this->attr('password'),
						  );

			  $client = new SoapClient($this->attr('wsdl'), $authen_info);

			// We need to convert the TRIM Execute object to XML, so we can swap the elements.
			$xml = $client->ConvertToXml($Execute);

			// Now we have to swap Fetch and RecordSearch element, because RecordSearch element has to
			// come first in the soap request, otherwise TRIM wont know where the Fetch object should get the
			// result from.
			if (isset($operations['Fetch']) && isset($operations['RecordSearch'])) {
				$xml = $this->SwapElement($xml->ConvertToXmlResult, 'Fetch', 'RecordSearch');
			} else if (isset($operations['Download']) && isset($operations['ShortcutRecordUri'])) {
				$xml = $this->SwapElement($xml->ConvertToXmlResult, 'Download', 'ShortcutRecordUri');
			} else {
				$xml = $xml->ConvertToXmlResult;
			}

			// Now its time to get the data
			$data = $client->ExecuteXml(Array('xml'	=> $xml));

		// The data returned is in XML format, so we have to parse it.
		$object_list = $this->parseTRIMData($data->ExecuteXmlResult, $display_type);

		if (isset($operations['Fetch'])) {
			return $object_list;
		} else if (isset($operations['Download'])) {
			//$res = $this->getPackagedFiles($object_list);
			$res = $this->getPackagedFiles($object_list, $hash_key, &$final_file_path);
		} else if (isset($operations['Upload'])) {
			if ($object_list['UploadId'] != $up_id) {
				echo 'Upload Successful, new UploadId: '.$object_list['UploadId'];
			}
		}//end else if

	}//end execute


	/**
	* Construct an array represent the RecordStringSearchClause object being sent
	*
	* @return unknown
	* @access public
	*/
	public function constructRecordStringSearchClause()
	{

		$search_type	= $this->getDynamicVariableValue('string_search_type');
		$search_clause	= Array (
							'Arg'	=> $this->getDynamicVariableValue('search_keyword'),
							'Type'	=> array_pop($search_type),
						  );

		return $search_clause;

	}//end constructRecordStringSearchClause()


	/**
	* Construct an array represent the RecordDateRangeSearchClause object being sent
	*
	* @return unknown
	* @access public
	*/
	public function constructRecordDateRangeSearchClause()
	{
		$start_time		= $this->getDynamicVariableValue('search_start_time');
		$end_time		= $this->getDynamicVariableValue('search_end_time');
		$type			= $this->getDynamicVariableValue('search_date_range_type');
		$search_clause	= Array (
							'StartTime'	=> $start_time,
							'EndTime'	=> $end_time,
							'Type'		=> $type,
						  );

		return $search_clause;

	}//end constructRecordDateRangeSearchClause()


	/**
	* Construct an array represent the RecordNumberRangeSearchClause object being sent
	*
	* @return unknown
	* @access public
	*/
	public function constructRecordNumberRangeSearchClause()
	{
		$start_number	= $this->getDynamicVariableValue('search_start_rec_num');
		$end_number		= $this->getDynamicVariableValue('search_end_rec_num');

		$search_clause	= Array(
							'Start'				=> $start_number,
							'End'				=> $end_number,
							'UseExpandedNumber'	=> TRUE,
						  );

		return $search_clause;

	}//end constructRecordNumberRangeSearchClause()


	/**
	* construct an array represent the RecordSearch object in the WSDL.
	*
	* @param string $keyword	the search term to be searched for
	* @param string $field		the field to be search in
	* @param array	$filetypes	file types
	* @param int	$limit		limit to how many record returned
	*
	* @return array
	* @access public
	*/
	public function constructRecordSearch()
	{

		$sort1	= $this->getDynamicVariableValue('search_sort_1');
		$sort2	= $this->getDynamicVariableValue('search_sort_2');
		$sort3	= $this->getDynamicVariableValue('search_sort_3');

		$search_kw = $this->getDynamicVariableValue('search_keyword');
		$string_search_type = $this->getDynamicVariableValue('string_search_type');

		$search_clauses = Array();
		if (!empty($search_kw) && !empty($string_search_type)) {
			$search_clauses['RecordStringSearchClause']	= $this->constructRecordStringSearchClause();
		}//end if

		$search_start_time	= $this->getDynamicVariableValue('search_start_time');
		$search_end_time	= $this->getDynamicVariableValue('search_end_time');

		if (!empty($search_start_time) && !empty($search_end_time)) {
			$search_clauses['RecordDateRangeSearchClause']	= $this->constructRecordDateRangeSearchClause();
		}//end if

		$search_start_rec_num	= $this->getDynamicVariableValue('search_start_rec_num');
		$search_end_rec_num		= $this->getDynamicVariableValue('search_end_rec_num');

		if (!empty($search_start_rec_num) && !empty($search_end_rec_num)) {
			$search_clauses['RecordNumberRangeSearchClause']	= $this->constructRecordNumberRangeSearchClause();
		}//end if

		$this->search_clauses = $search_clauses;

		$recordSearch	= Array (
							'Id'								=> 'recordsearch',
							'Sort1Descending'					=> $this->getDynamicVariableValue('sort_1_descending'),
							'Sort2Descending'					=> $this->getDynamicVariableValue('sort_2_descending'),
							'Sort3Descending'					=> $this->getDynamicVariableValue('sort_3_descending'),
							'TargetForUpdate'					=> FALSE,
							'IsForUpdate'						=> FALSE,
							'Limit'								=> $this->getDynamicVariableValue('limit'),
							'Sort1'								=> $sort1,
							'Sort2'								=> $sort2,
							'Sort3'								=> $sort3,
							//'Record'.$record_search_type.'Clause'	=> $search_clause,
							'FilterRecordTypesInclude'			=> '',
							'FilterRecordTypesExclude'			=> '',
							'FilterRecordClassInclude'			=> '',
							'FilterRecordClassExclude'			=> '',
							'FilterRecordDispositionInclude'	=> '',
							'FilterRecordDispositionExclude'	=> '',
							'FilterFileTypes' 					=> $this->getDynamicVariableValue('fileTypes'),
							'FilterFinalizedState' 				=> "Both",
							'Uri' 								=> '',
						);
		$recordSearch	= array_merge($recordSearch, $search_clauses);

		return $recordSearch;

	}//end constructRecordSearch()


	/**
	* construct an array to represent the ShortcutRecordUri object in the WSDL
	*
	* @param int $uri	The Unique Resource Identifier of each record
	*
	* @return void
	* @access public
	*/
	public function constructShortcutRecordUri($uri)
	{
		$sru	= Array (
					'Uri'				=> $uri,
					'TargetForUpdate'	=> TRUE,
					'IsForUpdate'		=> TRUE,
					'Limit'				=> 0,
				  );

		return $sru;

	}//end constructShortcutRecordUri()


	/**
	* construct an array to represent the Download object in the WSDL
	*
	* @param string	$trans_limit	The maximum transfer limit in bytes
	*
	* @return void
	* @access public
	*/
	public function constructDownload()
	{
		$download	= Array (
						'Checkout'				=> FALSE,
						'MaximumTransferBytes'	=> $this->attr('downloadLimit'),
						'TransferInset'			=> '0',
						'DownloadId'			=> 'chunk',
						'finished'				=> FALSE,
						'TargetForUpdate'		=> FALSE,
						'TransferType'			=> 'inline',
					  );

		return $download;

	}//end constructDownload()


	/**
	* construct an array to represent the Upload object in the WSDL
	*
	* @param string	$base64PayLoad	The base64 encoded content of the file.
	*
	* @return void
	* @access public
	*/
	public function constructUpload($base64PayLoad)
	{
		$upload		= Array (
						'TransferType'		=> 'inline',
						'Final'				=> TRUE,
						'Base64Payload'		=> $base64PayLoad,
						'TargetForUpdate'	=> TRUE,
					  );

		return $upload;

	}//end constructUpload()


	/**
	* construct an array to represent the Upload object in the WSDL
	*
	* @param string	$record_properties	The base64 encoded content of the file.
	*
	* @return void
	* @access public
	*/
	public function constructFetch($record_properties=Array())
	{
		$properties = $this->getDynamicVariableValue('fields');
		$spec_props = Array();
		foreach ($properties as $key => $value) {
			$spec_props[]	= Array (
								'Name'	=> $value,
							  );
		}//end foreach

		$fetch	= Array (
							'Id'				=> 'recordfetch',
							'IsForUpdate'		=> FALSE,
							'Items'				=> $spec_props,
							'TargetForUpdate'	=> FALSE,
							'Limit'				=> $this->getDynamicVariableValue('limit'),
							'Populate'			=> 0,
							'HideVersion'		=> FALSE,
						  );
		return $fetch;

	}//end constructFetch()


	/**
	* swap the two XML element that are adjacent to each other
	* The reason why we need this function is because sometimes RecordSearch need to be sent before Fetch, so Fetch know
	* where to fetch results from.
	*
	* @param string $xml	The TrimRequest object converted to XML
	* @param string $ele_1	First element (fetch)
	* @param string $ele_2	Second element (recordsearch)
	*
	* @return string
	* @access void
	*/
	private function SwapElement($xml, $ele_1, $ele_2)
	{
		$ele_1b = '<'.$ele_1.'>';
		$ele_1e = '</'.$ele_1.'>';
		$ele_2b = '<'.$ele_2.'>';
		$ele_2e = '</'.$ele_2.'>';

		$ele_1b_pos = strpos($xml, $ele_1b);
		$first_part = substr($xml, 0, $ele_1b_pos);
		$ele_1_part = substr($xml, $ele_1b_pos, strpos($xml, $ele_1e) + strlen($ele_1e) - $ele_1b_pos);

		$ele_2e_pos = strpos($xml, $ele_2e);
		$ele_2b_pos = strpos($xml, $ele_2b);

		$ele_2_part = substr($xml, $ele_2b_pos, $ele_2e_pos + strlen($ele_2e) - $ele_2b_pos);
		$second_part= substr($xml, $ele_2e_pos + strlen($ele_2e));

		$xml_result = $first_part . $ele_2_part . $ele_1_part . $second_part;

		return $xml_result;

	}//end SwapElement


	/**
	* This function parse the XML returned from the Request, depends on different operation, the result is different
	*
	* @param string	$data		The XML data returned from the Request
	* @param string	$operation	The major operation we are trying to achieve
	*
	* @return array
	* @access public
	*/
	private function parseTRIMData($data, $operation='Fetch')
	{
		$object_list = Array();
		$xml = new SimpleXMLElement($data);

		if ($operation == 'Fetch') {
			$i = 0;
			foreach ($xml->FetchResult->Objects->TrimObject as $object) {
				$uri = NULL;
				$version = NULL;
				foreach ($object->attributes() as $name => $attr_value) {
					if ((string)$name == 'Uri') $uri = (string)$attr_value;
					else if ($name = 'Version') {
						$version = (string)$attr_value;
					}
				}//end foreach

				$object_list[$i]	= Array (
										'Uri'		=> $uri,
										'Version'	=> $version,
									  );

				foreach ($object->Values->Value as $value) {
					$attr_type = NULL;
					$attr_val  = NULL;
					$error     = NULL;
					foreach ($value->attributes() as $name => $attr_value) {
						if ((string)$name == 'Name') {
							$attr_type = (string)$attr_value;
						}//end if
						if ((string)$name == 'ErrorMessage') {
							$error = (string)$attr_value;
						}//end if
						if ((string)$name == 'Val')	{
							$attr_val	= (string) $attr_value;
						}//end if
					}//end foreach
					if (!empty($error) && $error !== "false") {
						$object_list[$i]['error']	= $error;
						$object_list[$i]['FoundCount']	= (string) $xml->SearchResult->FoundCount;
					}
					$object_list[$i][$attr_type] = $attr_val;
				}//end foreach
			$i++;
			}//end foreach
		} else if ($operation == 'Download') {
			if (isset($xml->ErrorResult)) {
				$object_list['error']	= (string)$xml->ErrorResult->Message;
			} else {
				foreach ($xml->DownloadResult->children() as $key => $value) {
					$object_list[$key]	= (string)$value;
				}//end foreach
			}
		} else if ($operation == 'Upload') {
			if (isset($xml->ErrorResult)) {
				$object_list['error']	= (string)$xml->ErrorResult->Message;
			} else {
				foreach ($xml->UploadResult->children() as $key => $value) {
					$object_list[$key]	= (string)$value;
				}//end foreach
			}
		}

		return $object_list;

	}//end parseTRIMData()


	/**
	* this function send a header to let File be streamed via browser
	*
	* @param array $object_list	The array contain the information about the file and its content.
	*
	* @return void
	* @access public
	*/
	private function downloadFile($object_list)
	{
		if (!isset($object_list['error'])) {
			$file_content	= base64_decode($object_list['Base64Payload']);
			$file_name		= $object_list['AttachmentId'].'.'.strtolower($object_list['FileType']);
			$file_path		= '/home/hnguyen/matrix_installs/matrix_dev/data/temp/'.$file_name;

			if (!$handle	= fopen($file_path, 'w')) {
				log_dump('Could not open file '.$file_path);
				exit;
			}//end if
			if (fwrite($handle, $file_content) === FALSE) {
				log_dump('Could not write to file '.$file_path);
				exit;
			}//end if
			fclose($handle);

			// We'll be outputting a file
			header('Content-type: application/'.strtolower($object_list['FileType']));

			// Set the name of the file
			header("Content-Disposition: attachment; filename=$file_name");

			// Get the source file
			readfile($file_path);

			// Then remove the source file.
			unlink($file_path);

		} else {
			log_dump($object_list['error']);
		}//end if else

	}//end downloadFile()


	/**
	* this function download the file into the specified folder
	*
	* @param array $object_list	The array contain the information about the file and its content.
	*
	* @return boolean
	* @access public
	*/
	private function getPackagedFiles($object_list, $hash_key, &$final_file_path)
	{
		if (!isset($object_list['error'])) {
			$file_content	= base64_decode($object_list['Base64Payload']);
			$file_name		= $object_list['AttachmentId'].'.'.strtolower($object_list['FileType']);

			@mkdir(SQ_TEMP_PATH.'/'.$hash_key);
			$file_path		= SQ_TEMP_PATH.'/'.$hash_key.'/'.$file_name;

			$final_file_path = $file_path;
			if (!$handle	= fopen($file_path, 'w')) {
				log_dump('Could not open file '.$file_path);
			}//end if
			if (fwrite($handle, $file_content) === FALSE) {
				log_dump('Could not write to file '.$file_path);
			}//end if
			fclose($handle);

			return TRUE;
		} else {
			return FALSE;
		}//end else

	}//end getPackagedFiles($object_list)


	/**
	* this function return the record search types() available for searching
	*
	* @return array
	* @access public
	*/
	public function getRecordSearchTypes()
	{
		$rsTypes	= Array (
						'StringSearch'			=> 'Record Content',
						'DateRangeSearch'		=> 'Record Date Range',
						'NumberRangeSearch'		=> 'Record Number Range',
					  );

		return $rsTypes;
	}//end getRecordSearchTypes()


	/**
	* this function return the record string search clause types() available for searching
	*
	* @return array
	* @access public
	*/
	public function getRecordStringSearchClauseType()
	{
		$slTypes	= Array (
						'AnyWord'				=> 'AnyWord',
						'Consignment'			=> 'Consignment',
						'DocumentContent'		=> 'DocumentContent',
						'NotesWord'				=> 'NotesWord',
						'RecordNumber'			=> 'RecordNumber',
						'TitleWord'				=> 'TitleWord',
						'Caveat'				=> 'Caveat',
						'FileTypes'				=> 'FileTypes',
					  );

		return $slTypes;

	}//end getRecordStringSearchClauseType()


	/**
	* this function return the record date range search clause types() available for searching
	*
	* @return array
	* @access public
	*/
	public function getRecordDateRangeSearchClauseType()
	{
		$drTypes	= Array (
						'DateClosed'			=> 'DateClosed',
						'DateCreated'			=> 'DateCreated',
						'DateDue'				=> 'DateDue',
						'DateLastMoved'			=> 'DateLastMoved',
						'DateModified'			=> 'DateModified',
						'DateRegistered'		=> 'DateRegistered',
						'DueForReturn'			=> 'DueForReturn',
						'RequestedDate'			=> 'RequestedDate',
						'DateFinalized'			=> 'DateFinalized',
						'RetentionReviewDate'	=> 'RetentionReviewDate',
						'DatePublished'			=> 'DatePublished',
						'DateLastAction'		=> 'DateLastAction',
						'DateUpdated'			=> 'DateUpdated',
					  );

		return $drTypes;

	}//end getRecordDateRangeSearchClauseType()


	/**
	* this function return the record properties available for searching
	*
	* @return array
	* @access public
	*/
	public function getRecordProperties()
	{
		$properties = Array (
						'recAccessControl'					=> 'Access Control',
						'recAccessionNbr'					=> 'Accession Number',
						'recActions'						=> 'All Actions',
						'recAddresseeLoc'					=> 'Addressee',
						'recAllContacts'					=> 'All Contacts',
						'recAllHolds'						=> 'All Holds',
						'recAllMeetings'					=> 'Meetings (All)',
						'recAllParts'						=> 'All Parts',
						'recAllVersions'					=> 'All Versions',
						'recAltContainer'					=> 'Alternative Container',
						'recAltContainers'					=> 'Alternative Containers',
						'recAltContents'					=> 'Alternatively Contains',
						'recArchiveInterimDate'				=> 'Date Due for Interim Archival',
						'recArchiveLocalDate'				=> 'Date Due for Local Archival',
						'recArchivePermDate'				=> 'Date Due for Permanent Archival',
						'recAuthorLoc'						=> 'Author',
						'recAutoPartRule'					=> 'Automated Part Rule',
						'recBarcode'						=> 'TRIM Barcode',
						'recBlueprintTitle'					=> 'Blueprint Title',
						'recCheckedOutOn'					=> 'Checked Out On',
						'recCheckedOutPath'					=> 'Checked Out Path',
						'recCheckedOutTo'					=> 'Checked Out To',
						'recClassification'					=> 'Record Classification',
						'recClientLoc'						=> 'Client',
						'recConsignment'					=> 'Consignment',
						'recContainer'						=> 'Container',
						'recContents'						=> 'Contained Records',
						'recCreatorLoc'						=> 'Creator',
						'recCurrDisp'						=> 'Disposition',
						'recCurrentAction'					=> 'Current Action',
						'recCurrentLoc'						=> 'Assignee',
						'recCurrentLocationStatus'			=> 'Assignee Status',
						'recCurrentVersion'					=> 'Latest Finalized Version',
						'recDateAssigned'					=> 'Date Assigned',
						'recDateClosed'						=> 'Date Closed',
						'recDateCreated'					=> 'Date Created',
						'recDateDue'						=> 'Date Due',
						'recDateFinalized'					=> 'Date Declared As Final',
						'recDateInactive'					=> 'Date Inactive',
						'recDateModified'					=> 'Date Modified',
						'recDatePublished'					=> 'Date Published',
						'recDateReg'						=> 'Date Registered',
						'recDateUpdated'					=> 'Date Last Updated',
						'recDestructionDate'				=> 'Date Due for Destruction',
						'recDocumentAttachPending'			=> 'Document Attach is Pending',
						'recDocumentSize'					=> 'Size',
						'recDocumentType'					=> 'Document Type',
						'recDOSfile'						=> 'DOS file',
						'recEDetails'						=> 'Document Details',
						'recEnablerFlags'					=> 'Flags',
						'recEnclosed'						=> 'Enclosed?',
						'recESource'						=> 'Source Document',
						'recEStatus'						=> 'Edit Status',
						'recEStore'							=> 'Document Store',
						'recEStoreId'						=> 'Document Store ID',
						'recExtension'						=> 'Extension',
						'recExternalId'						=> 'External ID',
						'recFinalizeOnSave'					=> 'Declare As Final After Saving',
						'recForeignBarcode'					=> 'Foreign Barcode',
						'recFullClassification'				=> 'Full Classification Number',
						'recGeneratedTitle'					=> 'Title (Structured Part)',
						'recHasHold'						=> 'Is in one or more Holds',
						'recHasLinks'						=> 'Has Links',
						'recHistory'						=> 'Audit Events',
						'recHomeLoc'						=> 'Home Location',
						'recHomeLocationStatus'				=> 'Home Location Status',
						'recHomeSpace'						=> 'Home Space',
						'recIsCheckedOut'					=> 'Checked Out?',
						'recIsContainer'					=> 'Is a Container',
						'recIsElectronic'					=> 'Is Electronic',
						'recIsPart'							=> 'Is Part',
						'recIsRoot'							=> 'Is First Part',
						'recIsSigned'						=> 'Signature',
						'recKeywords'						=> 'All Thesaurus Terms',
						'recLastActionDate'					=> 'Last Action Date',
						'recLastPartRecord'					=> 'Latest Part',
						'recLatestVersion'					=> 'Latest Version',
						'recLongNumber'						=> 'Expanded Number',
						'recMeeting'						=> 'Meeting (Most Recent)',
						'recMovementHistory'				=> 'Movement History',
						'recNeedsDataEntryForm'				=> 'Needs Data Entry Form',
						'recNextPartRecord'					=> 'Next Part',
						'recNextTaskDue'					=> 'Next Scheduled Task Due',
						'recNotes'							=> 'Notes',
						'recNumber'							=> 'Record Number',
						'recNumberRenditions'				=> 'Number of Renditions',
						'recOtherLoc'						=> 'Other Contact',
						'recOverdueActions'					=> 'Overdue Actions',
						'recOwnerLoc'						=> 'Owner Location',
						'recPendingDispEvents'				=> 'Disposition Schedule',
						'recPreserveHierarchyOnDataEntry'	=> 'Preserve Hierarchy On Data Entry',
						'recPrevPartRecord'					=> 'Previous Part',
						'recPrimaryContactLoc'				=> 'Primary Contact',
						'recPriority'						=> 'Priority',
						'recRcClass'						=> 'Record Class',
						'recRecordType'						=> 'Record Type',
						'recRedactedFrom'					=> 'Redacted From',
						'recRedactionsOf'					=> 'All Redactions',
						'recRelatedRecord'					=> 'Related Record',
						'recRelatedRecs'					=> 'Related Records',
						'recRepresentLoc'					=> 'Representative',
						'recRequests'						=> 'All Record Requests',
						'recRetSchedule'					=> 'Retention Schedule',
						'recReturnDueDateTime'				=> 'Due for Return On',
						'recReviewDate'						=> 'Retention Review Date',
						'recRevisionNumber'					=> 'Revision Number',
						'recRootPartRecord'					=> 'First Part',
						'recSecurity'						=> 'Security',
						'recSeriesRecord'					=> 'Series Record',
						'recSuggestedFileName'				=> 'Suggested File Name',
						'recTerms'							=> 'Attached Thesaurus Terms',
						'recTitle'							=> 'Title',
						'recTopActions'						=> 'Top Actions',
						'recTypedTitle'						=> 'Title (Free Text Part)',
						'recViewPaneId'						=> 'View Pane ID',
					  );

		return $properties;

	}//end getRecordProperties()


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


}//end class
?>
