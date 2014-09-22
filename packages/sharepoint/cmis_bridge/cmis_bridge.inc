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
* $Id: cmis_bridge.inc,v 1.9 2012/08/30 00:59:19 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_PACKAGES_PATH.'/sharepoint/lib/cmis_common.inc';

/**
* CMIS Bridge
*
* Purpose
*	Creates file assets under itself in synchronization with a repository having a CMIS Connector such as MS Sharepoint.
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class CMIS_Bridge extends Asset
{

	/**
	* Holds the SOAP connections.
	* Reused for efficiency.
	*
	* @var array
	*/
	public $soap = NULL;

	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Pages ensure a name and short_name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', sprintf(translate('Unable to create a %s with a blank name'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link);

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

		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['file']   = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_2]['file']   = Array('card' => 'M', 'exclusive' => FALSE);

		return $allowed;

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{

		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), 0);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Connect and bind to the CMIS Server
	*
	* @param string  $service_name 	see CMIS_Bridge::getServices()
	* @param boolean $verify		whether to test the connection.
	*
	* @return boolean
	* @access public
	*/
	public function connect($service_name, $verify = FALSE)
	{

		if (!$verify && isset($this->soap[$service_name])) return TRUE;

		$cmis = new CMIS_Common();
		$wsdl = $this->attr('wsdl');
		$conn_info = Array(
						'username' => $this->attr('username'),
						'password' => $this->attr('password'),
						'ws_security' => $this->attr('ws_security'),
						'verify'	  => $verify,
						'service_name'=> $service_name
					);

		// Create and connect
		if (!$cmis->connect($wsdl[$service_name], $conn_info)) return FALSE;
		if (is_null($cmis->ptr)) return FALSE;

		$this->soap[$service_name] = $cmis;
		return TRUE;

	}//end connect()


	/**
	* Determines what needs to be updated, created and trashed. Used by the Sync CMIS Bridge HIPO Job.
	*
	* @return array
	* @access public
	*/
	public function synchronizeCheck()
	{

		$sync_info = Array(
						'update_assetids' 	=> Array(),
						'create_objectIds' 	=> Array(),
						'delete_assetids' 	=> Array()
					);

		$dont_delete = Array();

		//metadata info
		$current_files = $this->getCurrentFileInfo();

		//soap calls
		$repositoryId = $this->getRepositoryId();
		if (empty($repositoryId)) return $sync_info;

		$folderId = $this->getRootFolderId($repositoryId);
		if (empty($folderId)) return $sync_info;

		$children_data = $this->getFolderChildren($repositoryId, $folderId);
		if ($children_data === FALSE) return $sync_info;

		foreach ($children_data as $object_data){

			//if not document type or there is no contentStreamFileName this object is not applicable.
			if ($object_data['cmis:objectTypeId'] != 'cmis:document' || !isset($object_data['cmis:contentStreamFileName'])) continue;

			$objectId = $object_data['cmis:objectId'];
			if (array_key_exists($objectId, $current_files)) {
				$assetid = $current_files[$objectId]['assetid'];
				$dont_delete[] = $assetid;

				//check if needs updating
				if (isset($current_files[$objectId]['lastModificationDate'])){
					$lastModifiedCMIS = strtotime($object_data['cmis:lastModificationDate']);
					$lastModifiedMetadata = str_replace('/', '-', $current_files[$objectId]['lastModificationDate']);
					$lastModifiedMetadata = strtotime($lastModifiedMetadata);

					if ($lastModifiedCMIS > $lastModifiedMetadata){
						$sync_info['update_assetids'][] = Array(
																	'object_data' 			=> $object_data,
																	'assetid'				=> $assetid
																);
					}
				}
			} else {//mark for creation
				$sync_info['create_objectIds'][] = $object_data;
			}
		}

		//anything unmarked mark for deletion.
		foreach ($current_files as $objectid => $info){
			if (!in_array($info['assetid'], $dont_delete)) {
				$sync_info['delete_assetids'][] = $info['assetid'];
			}
		}

		return $sync_info;

	}//end synchronizeCheck()


	/**
	* Searches for all file assets located under the bridge and returns a mapping of the metadata sync values to the assetid.
	* Used for synchronization purposes. If no metadata value is present for the unique 'objectId', the file will be ignored.
	*
	* @return array
	* @access public
	*/
	public function getCurrentFileInfo()
	{
		$file_info = Array();

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$metadata_fields = $this->attr('metadata_fields');
		if (!empty($metadata_fields['cmis:objectId']['field'])) $sync_fields['objectId'] = $metadata_fields['cmis:objectId']['field'];
		if (!empty($metadata_fields['cmis:lastModificationDate']['field'])) $sync_fields['lastModificationDate'] = $metadata_fields['cmis:lastModificationDate']['field'];

		if (!isset($sync_fields['objectId'])) return $file_info;
		$field_names = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($sync_fields), 'metadata_field', FALSE, 'name');

		$files = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'file', FALSE);
		foreach (array_keys($files) as $file_id){
			$metadata_values = $mm->getMetadataFieldValues($file_id, array_values($field_names));
			$objectId = $metadata_values[$field_names[$sync_fields['objectId']]];
			if (!empty($objectId)) {
				$file_info[$objectId]['assetid'] = $file_id;
				if (isset($sync_fields['lastModificationDate'])){
					$lastModificationDate = $metadata_values[$field_names[$sync_fields['lastModificationDate']]];
					if (!empty($lastModificationDate)) $file_info[$objectId]['lastModificationDate'] = $lastModificationDate;
				}
			}
		}

		return $file_info;
	}//end getCurrentFileInfo()


	/**
	* Gets the available document fields of our document library that we can store in our metadata fields.
	* Mostly interested in custom fields but additionally a few CMIS fields used for synchronization.
	*
	* @return array
	* @access public
	*/
	public function getAvailableDocumentFields()
	{
		$fields = Array();
		if (!$this->connect('RepositoryService')) return $fields;

		$repositoryId = $this->getRepositoryId();
		if (empty($repositoryId)) return $fields;

		$type_def = $this->soap['RepositoryService']->getTypeDefinition($repositoryId, 'cmis:document');
		if(!isset($type_def->type->q1id) || (string)$type_def->type->q1id != 'cmis:document') return $fields;

		$properties = Array (
								'q1propertyBooleanDefinition',
								'q1propertyDateTimeDefinition',
								'q1propertyDecimalDefinition',
								'q1propertyIdDefinition',
								'q1propertyIntegerDefinition',
								'q1propertyStringDefinition'
							);

		$cmis_fields = Array (
								'cmis:objectId',
								'cmis:lastModificationDate',
								'cmis:createdBy',
								'cmis:creationDate'
							);
		foreach($properties as $propertyType) {
			if (isset($type_def->type->$propertyType)){
				$type = str_replace('Definition', '', substr($propertyType, 10));
				foreach($type_def->type->$propertyType as $property){
					$propertyId = (string)$property->q1id;
					if (strpos($propertyId, 'cmis:') !== FALSE && !in_array($propertyId, $cmis_fields)) continue;
					$fields[$propertyId] = Array (
															'display_name' => (string)$property->q1displayName,
															'type'		   => $type,
															'field'		   => ''
														);
				}
			}
		}

		return $fields;

	}//end getAvailableDocumentFields()


	/**
	* Gets the repository id based on the bridge 'Repository Name' attribute
	*
	* @return string
	* @access public
	*/
	public function getRepositoryId()
	{
		$repositoryId = '';

		if (!$this->connect('RepositoryService')) return $repositoryId;

		$repositories = $this->soap['RepositoryService']->getRepositories();
		if(!isset($repositories->repositories)) return $repositoryId;

		foreach($repositories->repositories as $repository){
			if ($repository->repositoryName == $this->attr('repository_name')){
				$repositoryId = (string)$repository->repositoryId;
				break;
			}
		}

		return $repositoryId;
	}//end getRepositoryId()


	/**
	* Given the repository id will return the root folder id
	*
	* @param string		$repositoryId
	*
	* @return string
	* @access public
	*/
	public function getRootFolderId($repositoryId)
	{
		$folderId = '';
		if (!$this->connect('RepositoryService')) return $folderId;

		$repositoryInfo = $this->soap['RepositoryService']->getRepositoryInfo($repositoryId);
		if (isset($repositoryInfo->repositoryInfo->rootFolderId)) $folderId = (string)$repositoryInfo->repositoryInfo->rootFolderId;

		return $folderId;
	}//end getRootFolderId()


	/**
	* Grabs all the documents located under the specified folder and returns all available information
	* Return FALSE on error.
	*
	* @param string		$repositoryId
	* @param string		$folderId
	*
	* @return boolean|array
	* @access public
	*/
	public function getFolderChildren($repositoryId, $folderId)
	{
		$children_data = Array();
		if (!$this->connect('NavigationService')) return FALSE;

		$children =  $this->soap['NavigationService']->getChildren($repositoryId, $folderId);
		if (isset($children->objects->numItems)){
			if ((int)$children->objects->numItems == 0) return $children_data;
		}
		if (!isset($children->objects->objects)) return FALSE;

		$key = 0;
		$properties = Array (
								'propertyBoolean',
								'propertyDateTime',
								'propertyDecimal',
								'propertyId',
								'propertyInteger',
								'propertyString'
							);
		foreach ($children->objects->objects as $object){
			foreach($properties as $propertyType) {
				if (isset($object->object->properties->$propertyType)){
					foreach($object->object->properties->$propertyType as $property){
						$attributes = $property->attributes();
						$children_data[$key][(string)$attributes['propertyDefinitionId']] = (string)$property->value;
					}
				}
			}
			$key++;
		}


		return $children_data;
	}//end getFolderChildren()


	/**
	* Gets contents of the specified object in binary form.
	*
	* @param string		$repositoryId
	* @param string		$objectId
	*
	* @return string
	* @access public
	*/
	public function getObjectContents($repositoryId, $objectId)
	{
		$binary_data = '';
		if (!$this->connect('ObjectService')) return $binary_data;

		$binary_data = $this->soap['ObjectService']->getContentStream($repositoryId, $objectId);
		return $binary_data;

	}//end getObjectContents()


}//end class
?>
