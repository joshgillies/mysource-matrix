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
* $Id: sharepoint_common.inc,v 1.11 2012/08/30 00:59:19 ewang Exp $
*
*/

/*
* Sharepoint Package Library public static functions
*
* This file contains global public static functions that are useful throughout the Sharepoint module
*
*
* @author Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/


class Sharepoint_Common
{


	/**
	* This is used to attach a file onto a sharepoint list item, and it can only be used on a custom list
	*
	* @return void
	* @access public
	*/
	public static function addAttachment($connection, $list_id, $list_item_id, $file_name, $base64Binary)
	{
		$list_wsdl	= self::getWSDLLocation('lists', $connection['url']);
		try {
			$client			= new SoapClient($list_wsdl, $connection['authentication']);
			$input_param	= Array (
								'listName'		=> $list_id,
								'listItemID'	=> $list_item_id,
								'fileName'		=> basename($file_name),
								'attachment'	=> $base64Binary,
							  );
			$response	= $client->AddAttachment($input_param);
			$upload_result	= $response->AddAttachmentResult;
			if (basename($upload_result) == basename($file_name)) {
				return TRUE;
			}//end if
		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch()

		return FALSE;

	}//end addAttachment()


	/**
	* This function is used to check out a file from Sharepoint / mark a file as checked out
	*
	* @return void
	* @access public
	*/
	public static function checkOutFile($page_url, $last_modified='')
	{
		try {
			$client			= new SoapClient(WSDL, $authen);
			$input_param	= Array (
								'pageUrl'			=> $page_url,
								'checkoutToLocal'	=> TRUE,
								'lastmodified'		=> $last_modified, //20 Jun 1982 12:00:00 GMT
							  );
			$response	= $client->CheckOutFile($input_param);
			$response_xml	= simplexml_load_string($response->GetPermissionCollectionResult->any);

		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch()

	}//end checkOutFile()


	/**
	* This function is used to return the WSDL location of a particular service
	*
	* @return string
	* @access public
	*/
	public static function getWSDLLocation($service_name, $url_root)
	{
		$services_list	= self::getServiceLocation();
		$clean_service_name	= str_replace(' ', '_', strtolower($service_name));

		if (isset($services_list[$clean_service_name]))
			return $url_root.$services_list[$clean_service_name].'?WSDL';
		else
			return NULL;

	}//end getWSDLLocation


	/**
	 * This function return the location of different sharepoint services
	 *
	 * @return array
	 * @access private
	 */
	private static function getServiceLocation()
	{
		$services_list	= Array (
							'administration'		=> '/_vti_adm/admin.asmx',
							'alerts'				=> '/_vti_bin/alerts.asmx',
							'document_workspace'	=> '/_vti_bin/dws.asmx',
							'form'					=> '/_vti_bin/forms.asmx',
							'imaging'				=> '/_vti_bin/imaging.asmx',
							'list_data_retrieval'	=> '/_vti_bin/dspsts.asmx',
							'lists'					=> '/_vti_bin/lists.asmx',
							'meeting'				=> '/_vti_bin/meeting.asmx',
							'permissions'			=> '/_vti_bin/permissions.asmx',
							'site_data'				=> '/_vti_bin/sitedata.asmx',
							'site'					=> '/_vti_bin/sites.asmx',
							'usergroup'				=> '/_vti_bin/usergroup.asmx',
							'versions'				=> '/_vti_bin/versions.asmx',
							'web_part_pages'		=> '/_vti_bin/webpartpages.asmx',
							'webs'					=> '/_vti_bin/webs.asmx',
						  );

		return $services_list;

	}//end getServiceLocation()


	/**
	 * This function return the information about a particular user
	 *
	 * @param string $user_name	The username of the user we are getting information for
	 *
	 * @return array
	 * @access public
	 */
	public static function getUserInfo($user_name='administrator')
	{

		try {
			$client			= new SoapClient(WSDL_USRGRP, $authen);
			$input_param	= Array (
								'userLoginName'	=> $user_name,
							  );
			$response	= $client->GetUserInfo($input_param);
			$response_xml	= simplexml_load_string($response->GetPermissionCollectionResult->any);

		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch()

	}//end getUserInfo()


	/**
	 * This function return the permission collection for a list or a site
	 *
	 * @param string $list_name	The name of the list we are trying to get the permission collection
	 *
	 * @return array
	 * @access public
	 */
	public static function getPermissionCollection($list_name)
	{
		try {
			$client	= new SoapClient(WSDL_PERM, $authen);
			$input_param	= Array (
								'objectName'	=> $list_name,
								'objectType'	=> 'List',
							  );
			$response	= $client->GetPermissionCollection($input_param);
			$response_xml	= simplexml_load_string($response->GetPermissionCollectionResult->any);

		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch()

	}//end getPermissionCollection()


	/**
	* Delete a sharepoint list
	*
	* @param string $list_name	The name of the list being deleted
	*
	* @return boolean
	* @access public
	*/
	public static function deleteList($list_name)
	{
		global	$authen;
		try {
			$client	= new SoapClient(WSDL, $authen);
			$input_param	= Array (
								'listName'		=> $list_name,
							  );
			$response	= $client->DeleteList($input_param);
		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch

	}//end deleteList()


	/**
	* This function add a new List into sharepoint
	*
	* @param string $list_name		The name of the list being added
	* @param string $description	The description for the list
	* @param string $templateID		The templateID from the sharepoint system
	*
	* @return boolean
	* @access public
	*/
	public static function addList($list_name, $description='', $templateID='100')
	{
		global	$authen;
		try {
			$client	= new SoapClient(WSDL, $authen);
			$input_param	= Array (
								'listName'		=> $list_name,
								'description'	=> $description,
								'templateID'	=> $templateID,
							  );
			$response	= $client->AddList($input_param);
		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch

	}//end addList()


	/**
	* This function return a list of templates in sharepoint and their IDs
	* When creating a new list, user would have to base the new list on on of these template
	*
	* @param string $template_name	The name of the template we are trying to get the templateID for
	*
	* @return array|int
	* @access public
	*/
	private static function getTemplateId($template_name='')
	{
		$template_ids	= Array (
							'announcements'					=> '104',
							'contacts'						=> '105',
							'custom_list'					=> '100',
							'custom_list_in_datasheet_view'	=> '120',
							'dataSources'					=> '110',
							'discussion_board'				=> '108',
							'document_library'				=> '101',
							'events'						=> '106',
							'form_library'					=> '115',
							'issues'						=> '1100',
							'links'							=> '103',
							'picture_library'				=> '109',
							'survey'						=> '102',
							'tasks'							=> '107',
						  );

		if (!empty($template_name)) {
			return $template_ids[$template_name];
		} else {
			return $template_ids;
		}//end else

	}//end getTemplateId()


	/**
	* This function return the details of a list.
	* The details would include all the attributes and fields of the list, this is useful when trying to create
	* a new list item in a list that its structure if unknown.
	*
	* @param array	$connection	The connection information
	* @param string $list_id	The ID of the list we are trying to get information for
	*
	* @return string
	* @access public
	*/
	public static function getList($connection, $list_id)
	{
		$list_wsdl	= self::getWSDLLocation('lists', $connection['url']);
		try {
			$client			= new SoapClient($list_wsdl, $connection['authentication']);
			$input_param	= Array (
								'listName'	=> $list_id,
							  );
			$response	= $client->getList($input_param);

		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
			return '';
		}//end try catch()

		return $response;

	}//end getList()


	/**
	* This function return all the available fields for a particular list
	*
	* @param array		$connection		The connection information
	* @param string		$list_id		The ID of the list we are trying to get information for
	* @param bool		$all			Get all fields or non-read-only fields
	* @param bool		$get_field_info	Get fields information
	*
	* @return array
	* @access public
	*/
	public static function getListAvailableFields($connection, $list_id, $all=FALSE, $get_field_info=FALSE)
	{
		$response	= self::getList($connection, $list_id);

	    $available_fields   = Array();
	    $field_info			= Array();
		if (!empty($response)) {
		    $xml    = simplexml_load_string($response->GetListResult->any);
		    if (!empty($xml)) {
			    foreach ($xml->Fields->children() as $field) {
			        $read_only      = (string) $field['ReadOnly'];
			        if ($read_only == 'TRUE' && $all == FALSE) continue;

			        if (!$get_field_info) {
				        $field_xml  = $field->asXML();
				        $pattern    = '/(<Field Type[^>]*>)/s';
				        $matches	= Array();
				        preg_match($pattern, $field_xml, $matches);
				        if (isset($matches[0]) && !empty($matches[0])) {
				            $available_fields[(string) $field['Name']]      = (string) $field['DisplayName'];
				        } else {
				            $available_fields[(string) $field['Name']]   = (string) $field['DisplayName'];
				        }
			        } else {
				        foreach ($field->attributes() as $attr_name => $attr_val) {
							$field_info[(string) $field['Name']][$attr_name]	= (string) $attr_val;
				        }//end if
			        }//end else
			    }//end foreach
		    }//end if
		}//end if

		if ($get_field_info) {
			return $field_info;
		} else {
	    	asort($available_fields);
			return $available_fields;
		}//end else

	}//end getListAvailableFields()


	/**
	* This function delete a list item from a list
	*
	* @param string	$list_id		The ID of the list we are deleting items from
	* @param string $list_item_id	The item ID we are trying to delete
	*
	* @return boolean
	* @access public
	*/
	public static function deleteListItem($list_id, $list_item_id)
	{

		try {
			$client	= new SoapClient(WSDL, $authen);
			$update_xml		= '
<Batch OnError="Continue" PreCalc="TRUE"
ListVersion="0" >
   <Method ID="1" Cmd="Delete">
      <Field Name="ID">'.$list_item_id.'</Field>
   </Method>
</Batch>
';
			$input_param	= Array (
								'listName'	=> $list_id,
								'updates'	=> Array (
												'any'	=> $update_xml,
											   ),
							  );
			$response	= $client->UpdateListItems($input_param);

		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch

	}//end deleteListItem()


	/**
	* This function add a list item to a list (preferably a custom list)
	*
	* @param array	$connection			The connection information
	* @param string $list_id			The ID of the list we are trying to get information for
	* @param array	$sharepoint_fields	The fields we are specifying values for
	*
	* @return boolean
	* @access public
	*/
	public static function addListItem($connection, $list_id, $sharepoint_fields)
	{

		$list_wsdl	= self::getWSDLLocation('lists', $connection['url']);
		try {
			$client	= new SoapClient($list_wsdl, $connection['authentication']);
			$fs		= $client->__getFunctions();

			$inner_xml	= '';
			foreach ($sharepoint_fields as $field_id => $value) {
				if (!empty($value)) {
					$inner_xml	.= '<Field Name="'.$field_id.'">'.$value.'</Field>';
				}//end if
			}//end foreach

			$request_xml	= '<Batch OnError="Continue" PreCalc="TRUE" ListVersion="0" ><Method ID="1" Cmd="New">'.$inner_xml.'</Method></Batch>';

			$input_param	= Array (
								'listName'	=> $list_id,
								'updates'	=> Array (
												'any'	=> $request_xml,
											   ),
							  );

			$response	= '';
			$response	= $client->UpdateListItems($input_param);

		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);

		}//end try catch

		$xml		= simplexml_load_string($response->UpdateListItemsResult->any);
		$error_code	= (string) $xml->Result->ErrorCode;

		if ($error_code == '0x00000000') {
		    $items_xml  = $xml->Result;
		    // zz is used for the namespace z in the xml
		    $items_xml->registerXPathNamespace('zz', '#RowsetSchema');
		    $zresults   = $items_xml->xpath('//zz:row');

			$ows_ID		= (int) $zresults[0]['ows_ID'];
		    return $ows_ID;
		} else {
			$res_msg	= 'Error Code '.$error_code .' - '.(string) $xml->Result->ErrorText;
			return $res_msg;
		}//end else

		return TRUE;

	}//end addListItems()


	/**
	* This function construct the XML search query for sharepoint
	*
	* @param array	$search_query	The search criterion
	* @param string	$search_logic	AND or OR
	* @param array	$field_info		The information about all fields available for this list
	*
	* @return string
	* @access private
	*/
	private static function constructXMLQuery($search_query, $search_logic, $field_info)
	{
		// Refer to MS CAML Query schema @ http://msdn.microsoft.com/en-us/library/ms467521.aspx

		$query_str	= '';
		$operators	= Array (
						'<'				=> 'Lt',
						'<='			=> 'Leq',
						'>'				=> 'Gt',
						'>='			=> 'Geq',
						'='				=> 'Eq',
						'!='			=> 'Neq',
						'Contains'		=> 'Contains',
						'BeginsWith'	=> 'BeginsWith',
						'IsNotNull'		=> 'IsNotNull',
						'IsNull'		=> 'IsNull',
					  );

		$search_query_temp	= Array();

		foreach ($search_query as  $index => $field_crit_info) {
			$field_name	= key($field_crit_info);
			$crit_info	= $field_crit_info[$field_name];
			$op 		= $operators[$crit_info['operator']];
			$val		= $crit_info['value'];
			$type		= $field_info[$field_name]['Type'];

			$query_str	= '<'.$op.'><FieldRef Name="'.$field_name.'" /><Value Type="'.$type.'">'.$val.'</Value></'.$op.'>';
			$search_query_temp[]	= $query_str;
		}//end if

		$query_strings		= self::recursiveXMLConstruct($search_query_temp, $search_logic);
		$built_query_string	= array_pop($search_query_temp);

		$final_query_str	= "<Query><Where>".$built_query_string."</Where></Query>";

		return $final_query_str;

	}//end constructXMLQuery()


	/**
	* This function construct the XML for CAML query recursively
	*
	* @param array	&$search_query	The array which contain query clauses inside, to be grouped recursively
	* @param string	$search_logic	The search logic AND or OR
	*
	* @return void
	* @access private
	*/
	private static function recursiveXMLConstruct(&$search_query, $search_logic)
	{
		/* Ok, a bit confusing here, but we want to add an logic (AND/ OR) to a pair of conditions, if we have an
		* uneven number of conditions, say 3, we would need to have something like
		* <AND>
		* 	<AND><ConditionA><ConditionB></AND>
		* 	<ConditionC>
		* </AND>
		* If it is even then it's simpler
		* <AND>
		* 	<AND><ConditionA><ConditionB></AND>
		* 	<AND><ConditionC><ConditionD></AND>
		* </AND>
		*/
		$temp_array	= Array();
		for (reset($search_query); NULL !== ($k = key($search_query)); next($search_query)) {
			$current_element = $search_query[$k];
			if (!empty($current_element)) {
				$next_element	= next($search_query);
				 if (!empty($next_element)) {
					$search_clause	= '<'.$search_logic.'>'.$current_element.$next_element.'</'.$search_logic.'>';
					$temp_array[]	= $search_clause;
				 } else {
					$search_clause	= $current_element;
					$temp_array[]	= $search_clause;
				 }//end else
			}//end if
		}//end for
		$search_query	= $temp_array;
		if (count($search_query) >= 2) {
			self::recursiveXMLConstruct($search_query, $search_logic);
		}//end if

	}//end recursiveXMLConstruct


	/**
	* This function is used to search for list items within a sharepoint list
	* An XML query can be provided to set the criterions of the search based on item's fields
	*
	* @param $search_info array	The fields criterions to search for
	*
	* @return array
	* @access public
	*/
	public static function getListItems($connection, $list_id, $search_query=Array(), $search_logic='And', $row_limit='', $download_file=FALSE, $bridge_info="")
	{
		$result	= Array();
		$list_wsdl	= self::getWSDLLocation('lists', $connection['url']);
		try {
			$client			= new SoapClient($list_wsdl, $connection['authentication']);

			$ndViewFields	= "	<viewFields>
									<FieldRef Name='Field1'/><FieldRef Name='Field2'/>
								</viewFields>";

			$param			= Array (
								'listName'		=> $list_id,
								'viewName'		=> '',
								'rowLimit'		=> $row_limit,
							  );
			$ndQueryOptions	= '	<QueryOptions>
									<IncludeAttachmentUrls>TRUE</IncludeAttachmentUrls>
								</QueryOptions>';

			$param['queryOptions']	= Array (
										'any'	=> $ndQueryOptions,
									  );

			if (!empty($search_query)) {
				$field_info	= self::getListAvailableFields($connection, $list_id, TRUE, TRUE);
				$query	= self::constructXMLQuery($search_query, $search_logic, $field_info);
				$param['query']	= Array (
									'any'	=> $query,
								  );

			}//end if

			$items		= $client->GetListItems($param);
			$all_items	= $items->GetListItemsResult->any;

			$items_xml	= simplexml_load_string($all_items);
			$namespaces = $items_xml->getDocNamespaces();
			$items_xml->registerXPathNamespace('zz', '#RowsetSchema');
			$zresults	= $items_xml->xpath('//zz:row');

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			foreach ($zresults as $zresult) {
				$ows_ID				= (string) $zresult['ows_ID'];
				foreach ($zresult->attributes() as $attr_name => $attr_val) {
					$attr_val	= str_replace($ows_ID.';#', '', $attr_val);
					if ($attr_name == 'ows_Attachments') {
						$attr_val	= str_replace(';#', '', $attr_val);
					}//end
					$result[$ows_ID][$attr_name]	= (string) $attr_val;
				}//end foreach
				
				//url encode the spaces in file url
				if(isset($result[$ows_ID]['ows_FileRef']))
					$result[$ows_ID]['ows_FileRef'] = str_replace(' ', '%20', $result[$ows_ID]['ows_FileRef']);
				if(isset($result[$ows_ID]['ows_Attachments']))
					$result[$ows_ID]['ows_Attachments'] = str_replace(' ', '%20', $result[$ows_ID]['ows_Attachments']);
				
				
				$url_bits = parse_url($connection['url']);
				$base_url = $url_bits['scheme'].'://'.$url_bits['host'];
				if (isset($url_bits['port']) && !empty($url_bits['port'])) $base_url = $base_url.':'.$url_bits['port'];
				$remote_download_url = $base_url.'/'.$result[$ows_ID]['ows_FileRef'];

				// If files needs to served locally
				if ($download_file) {
					$data_path = $bridge_info['data_path'].'/'.$list_id.'/';
					if (create_directory($data_path)) {
						self::downloadFiles(Array($remote_download_url), $data_path, $connection['authentication']);
						$result[$ows_ID]['download_url'] = $bridge_info['url'].'?file_uri='.$list_id.'/'.basename($result[$ows_ID]['ows_FileRef']);
					} else {
						trigger_error('Unable to download the file to directory: '.$data_path, E_USER_WARNING);
						$result[$ows_ID]['download_url'] = '';
					}
				} else {
					$result[$ows_ID]['download_url'] = $remote_download_url;
				}
				
				// Also download the file attachment to local
				if ($download_file && isset($result[$ows_ID]['ows_Attachments']) && $result[$ows_ID]['ows_Attachments']) {
					$data_path = $bridge_info['data_path'].'/'.$list_id.'/';
					if (create_directory($data_path)) {
						self::downloadFiles(Array($result[$ows_ID]['ows_Attachments']), $data_path, $connection['authentication']);
						$result[$ows_ID]['ows_Attachments'] = $bridge_info['url'].'?file_uri='.$list_id.'/'.basename($result[$ows_ID]['ows_Attachments']);
					} else {
						trigger_error('Unable to download the file attachment to directory: '.$data_path, E_USER_WARNING);
						$result[$ows_ID]['ows_Attachments'] = '';
					}
				} 
				
			}//end foreach

		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
			trigger_error($sf->getMessage().'.'.$detail, E_USER_WARNING);
		}//end try catch

		return $result;

	}//end getListItems()


	/**
	 * This function is used return all the list available in the system
	 *
	 * @return array
	 * @access public
	 */
	public static function getListCollections($connection, $custom_list_only=FALSE)
	{
		$lists	= Array();
		$list_wsdl	= self::getWSDLLocation('lists', $connection['url']);
		try {
			$client			= new SoapClient($list_wsdl, $connection['authentication']);
			$collections = $client->GetListCollection();

			$all_lists	= $collections->GetListCollectionResult->any;
			$result		= simplexml_load_string($all_lists);

			foreach ($result->children() as $list) {
				$list_id	= (string) $list ['ID'];
				if ($custom_list_only && (string) $list['ServerTemplate'] != '100') {
					continue;
				}//end if
				foreach ($list->attributes() as $name	=> $value) {
					$lists[$list_id][$name]	= (string) $value;
				}//end foreach

			}//end foreach
		} catch (SoapFault $sf) {
			$detail = '';
			if (isset($sf->detail)) $detail = $sf->detail;
		}//end try catch

		return $lists;

	}//end getListCollections()


	/**
	* This function get all list name from Sharepoint
	*
	* @param array $connection			The connection information
	* @param array $all_list_details	The list details, if already gotten from somewhere else
	*
	* @return array
	* @access public
	*/
	public static function getListNames($connection, $all_list_details=Array())
	{
		if (empty($all_list_details)) {
			$all_list_details	= self::getListCollections($connection);
		}//end if
		$list_names			= Array();

		foreach ($all_list_details as $list_id => $list_info) {
			$list_names[$list_id]	= $list_info['Title'];
		}//end if

		return $list_names;

	}//end getListNames()


    /**
    * This function use CURL to download all the files from the provided URLs
    *
    * @param array	$urls		the array of document urls to download.
    * @param string	$save_path	the destination to save to
    * @param string	$auth		http authentication
    *
    * @return int		// The number of file(s) downloaded
    * @access public
    */
    private static function downloadFiles($urls, $save_path, $auth)
    {
		$mh	= curl_multi_init();
		foreach ($urls as $i => $url) {
			$final_path	= $save_path.urldecode(basename($url));
			$conn[$i]	= curl_init($url);
			$fp[$i]		= fopen($final_path, "w");
			curl_setopt($conn[$i], CURLOPT_FILE, $fp[$i]);
			curl_setopt($conn[$i], CURLOPT_HEADER ,0);
			curl_setopt($conn[$i], CURLOPT_CONNECTTIMEOUT, 60);
			curl_setopt($conn[$i], CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
			if (isset($auth['login']) && isset($auth['password']))
				curl_setopt($conn[$i], CURLOPT_USERPWD, $auth['login'].':'.$auth['password']);
			curl_multi_add_handle($mh,$conn[$i]);
		}//end foreach

		do {
			$n = curl_multi_exec($mh, $active);
		}//end do
		while ($active);
		foreach ($urls as $i => $url) {
			if (isset($conn[$i]) && isset($fp[$i])) {
				curl_multi_remove_handle($mh, $conn[$i]);
				curl_close($conn[$i]);
				fclose($fp[$i]);
			}//end if
		}//end foreach
		curl_multi_close($mh);

    }//end downloadFiles()


	/**
	* This function test whether a connection is valid
	*
	* @param array $connection	The connection information
	*
	* @return boolean
	* @access public
	*/
	public static function test_sharepoint_connection($connection, $silent=FALSE)
	{

		//test a cURL connection first otherwise the Soap constructor will crash and leave edit interfaces inaccessible or cause Cron deadlocks.
		if (!isset($connection['authentication']['proxy_login'])){ //TODO: test cURL with proxies involved
			$options = array(
						'FOLLOWLOCATION' => 1,
						'RETURNTRANSFER' => 1,
						'HTTPAUTH'          => CURLAUTH_BASIC,
						'TIMEOUT'        => 5,
						'SSL_VERIFYPEER' => 0,
						'USERPWD'        => $connection['authentication']['login'].":".$connection['authentication']['password'],
					   );
			$details  = fetch_url($connection['url'], $options, array(), FALSE);
			bam($details);
			$contents = $details['response'];
			if (strpos($contents, 'wsdl') === FALSE) return FALSE;
		}


		try {
			$client = new SoapClient(self::getWSDLLocation('lists', $connection['url']), $connection['authentication']);
			$fns	= $client->__getFunctions();

	        if (!empty($fns)) {
				$all_lists	= Sharepoint_Common::getListCollections($connection);
				if (empty($all_lists)) return FALSE;
	        	return TRUE;
	        } else {
	        	return FALSE;
	        }//end else

		} catch (SoapFault $e) {
			if (!$silent) {
				echo $e->getMessage().'<br />';
			}//end if
			return FALSE;
		}//end if

	}//end test_trim_connection()



}//end class
