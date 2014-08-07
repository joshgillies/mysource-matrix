<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_metadata_service.inc,v 1.12 2013/07/26 08:50:50 cupreti Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Metadata_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Metadata_Service extends Soap_Api
{


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns a function list of the asset let the children handle this
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function getFunctionList()
	{
		return Array(
				'SetMetadataSchema'				=> '1',
				'GetMetadataFieldsOfSchema'		=> '1',
				'GetMetadataFieldsDetailOfSchema'		=> '1',
				'GetMetadataFieldValues'		=> '1',
				'RegenerateMetadataSchema'		=> '1',
				'RegenerateMetadataAsset'		=> '1',
				'GetMetadataValueByIDs'			=> '1',
				'SetAssetMetadata'				=> '1',
				'SetMultipleMetadataFields'		=> '1',
				'SetMetadataFieldDefaultValue'	=> '1',
				'GetSchemasOnAsset'				=> '1',
			   );

	}//end getFunctionList()


	/**
	* Description: This operation will apply or revoke a metadata schema from an asset
	*
	* @param array  $request	The request information
	* <pre>
	* Array (
	*		 'AssetID'	 	=> [The asset to apply the schema on],
	*		 'SchemaID'		=> [The metadata scheme being applied],
	*		 'Grant'		=> [Apply, Deny, Revoke],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function SetMetadataSchema($request)
	{
		$request_info	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$schemaid			= array_get_index($request_info, 'SchemaID', '');
		$grant				= array_get_index($request_info, 'Grant', '');	// Apply, Deny, Revoke = 1, 0, -1
		$grant_level		= Array (
								'Apply'		=> '1',
								'Deny'		=> '0',
								'Revoke'	=> '-1',
							  );
		$grant_val			= $grant_level[$grant];	// We are not going check if isset index, the WSDL should check the input values for $grant
		$cascade			= array_get_index($request_info, 'Cascade', '1');
		$dependants			= $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);
		$has_dependants		= !empty($dependants);

		if (!empty($assetid) && !empty($schemaid)) {
			$hh				= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$running_vars	= Array (
								'schema_changes'	=> Array (
														Array (
															'assetids'			=> Array ( $assetid),
															'granted'			=> $grant_val,
															'schemaid'			=> $schemaid,
															'cascades'			=> $cascade,
															'previous_access'	=> NULL,
														),
													   ),
							  );

			$errors	= $hh->freestyleHipo('hipo_job_edit_metadata_schemas', $running_vars);
			if (empty($errors)) {
				return Array (
						'SetSchemaResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable to set Metadata Schema on Asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID or Schema ID is not valid. Please provide a valid AssetID and SchemaID');
		}//end else

	}//end SetMetadataSchema()


	/**
	* Description: This operation will regenerate metadata for all asset which has a metadata schema applied on
	*
	* @param array  $request	The request information
	* <pre>
	* Array (
	*		 'SchemaID'		=> [The metadata scheme being applied],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function RegenerateMetadataSchema($request)
	{
		$request_info	= (Array) $request;
		$schemaid			= array_get_index($request_info, 'SchemaID', '');

		if (!empty($schemaid)) {
			$schema	= $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (!($schema instanceof Metadata_Schema)) {
				throw new SoapFault('Server', 'The provided Schema ID does not belong to a metadata schema');
			}//end if
			$hh				= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$running_vars	= Array (
								'schemaids'	=> Array ($schemaid),
							  );
			$errors	= $hh->freestyleHipo('hipo_job_regenerate_metadata', $running_vars);
			if (empty($errors)) {
				return Array (
						'RegenerateMetadataSchemaResult'	=> TRUE,
					   );
			} else {
				$message = '';
				if (isset($errors[0]['message'])) $message	= $errors[0]['message'];
				throw new SoapFault('Server', 'Unable To Regenerate Metadata For Schema. '.$message);
			}//end else
		} else {
			throw new SoapFault('Server', 'Schema ID Is Not Valid. Please Provide A Valid Schema ID');
		}//end else

	}//end RegenerateMetadataSchema()


	/**
	* Description: This operation will regenerate metadata for the specified asset
	*
	* @param array  $request	The request information
	* <pre>
	* Array (
	*		 'AssetID'		=> [The asset to regenerate metadata for],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function RegenerateMetadataAsset($request)
	{
		$request_info	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');

		if (!empty($assetid)) {
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'metadata');
			$result	= $mm->regenerateMetadata($assetid, 'all');
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'metadata');
			if ($result) {
				return Array (
						'RegenerateMetadataAssetResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Regenerate Metadata For Asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID Is Not Valid. Please Provide A Valid Asset ID');
		}//end else

	}//end RegenerateMetadataAsset()


	/**
	* Description: This operation will set the value for a metadata field of an asset
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		 'AssetID'		=> [The asset to regenerate metadata for],
	*		 'FieldID'		=> [The metadata field id],
	*		 'NewValue'		=> [The new value for the field],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function SetAssetMetadata($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$fieldid		= array_get_index($request_info, 'FieldID', '');
		$new_value		= array_get_index($request_info, 'NewValue', '');

		if (!empty($assetid) && !empty($fieldid)) {
			$field	= $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
			if (!($field instanceof Metadata_Field)) {
				throw new SoapFault('Server', 'The Field ID provided does not belong to a metadata field');
			}//end if
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'metadata');

			$field_asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
			$field_name		= $field_asset->attr('name');

			// Encode value before setting metadata
			$new_value = Metadata_Field::encodeValueString($new_value, Array());

			$metadata_info	= Array (
								$fieldid	=> Array (
												Array (
													'name'	=> $field_name,
													'value'	=> $new_value,
												),
											   ),
							  );
			$result	= $mm->setMetadata($assetid, $metadata_info);
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'metadata');
			if ($result) {
				return Array (
						'SetAssetMetadataResult'	=> $result,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Regenerate Metadata For Asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Please provide both AssetID and FieldID of the metadata field.');
		}//end else

	}//end SetAssetMetadata()


	/**
	* Description: This operation will set the value for a metadata fields of an asset
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		 'AssetID'		=> [The asset to regenerate metadata for],
	*		 'MetadataInfo'	=> [metadata field id/value pairs],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function SetMultipleMetadataFields($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$metadata_info	= array_get_index($request_info, 'MetadataInfo', Array());

		if(!is_array($metadata_info))
			$metadata_info = Array($metadata_info);

		if (!empty($assetid) && !empty($metadata_info)) {
			$metadata_values = Array();
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			foreach ($metadata_info as $metadata) {
				$metadata = (Array) $metadata;

				$field	= $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata['FieldID']);

				if (!($field instanceof Metadata_Field)) {
					throw new SoapFault('Server', 'Field ID provided, '.$metadata['FieldID'].', does not belong to a metadata field');
				}//end if


				$field_name		= $field->attr('name');

				// Encode value before setting metadata
				$new_value = Metadata_Field::encodeValueString($metadata['FieldValue'], Array());

				$metadata_values[$field->id] = Array (
													Array (
														'name'	=> $field_name,
														'value'	=> $new_value,
													),
											   );
			}//end foreach

			$GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'metadata');
			$result	= $mm->setMetadata($assetid, $metadata_values);
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'metadata');

			if ($result) {
				return Array (
						'SetMultipleMetadataFieldsResult'	=> $result,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Set Metadata For Asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Please provide both AssetID and Metadata Field id/value info.');
		}//end else

	}//end SetMultipleMetadataFields()


	/**
	* Description: This operation will set the default value for a metadata field
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		 'FieldID'			=> [The metadata field id],
	*		 'NewDefaultValue'	=> [The new default value for the field],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function SetMetadataFieldDefaultValue($request)
	{
		$request_info	= (Array) $request;
		$fieldid		= array_get_index($request_info, 'FieldID', '');
		$new_def_value	= array_get_index($request_info, 'NewDefaultValue', '');

		if (!empty($new_def_value) && !empty($fieldid)) {
			$field		= $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);
			if (!($field instanceof Metadata_Field)) {
				throw new SoapFault('Server', 'The provided Field ID does not belong to a Metadata Field');
			}//end if
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();

			$GLOBALS['SQ_SYSTEM']->am->acquireLock($fieldid, 'attributes');

			$field->setAttrValue('default', $new_def_value);
			$result	= $field->saveAttributes();

			$GLOBALS['SQ_SYSTEM']->am->releaseLock($fieldid, 'attributes');
			if ($result) {
				return Array (
						'SetMetadataFieldDefaultValueResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Set New Default Value For Metadata Field');
			}//end else
		} else {
			throw new SoapFault('Server', 'Please Provide Both FieldID And New Default Value For The Field');
		}//end else

	}//end SetMetadataFieldDefaultValue()


	/**
	* Description: This operation will return the metadata value for a metadata field of an asset
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		 'AssetID'			=> [The Asset in query],
	*		 'FieldID'			=> [The metadata field ID],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function GetMetadataValueByIDs($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$fieldid		= array_get_index($request_info, 'FieldID', '');

		if (!empty($assetid) && !empty($fieldid)) {
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$values = $mm->getMetadataValueByAssetid($assetid, $fieldid);
			if (!empty($values)) {
				// bug fix #4646 Empty response returned from metadata field containing non-ascii character
				// we will need to escape the characters if we are not on a utf-8 system
				return Array (
						'GetMetadataValueByIDsResult'	=> $mm->escapeMetadata($values),
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Get Metadata Values For Asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Please provide both AssetID and FieldID of the metadata field.');
		}//end else

	}//end GetMetadataValueByIDs()


	/**
	* Description: This operation will return the metadata schemas applied on the asset
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		 'AssetID'			=> [The Asset in query],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function GetSchemasOnAsset($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');

		if (!empty($assetid)) {
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$schemaids = $mm->getSchemas($assetid, NULL, NULL, TRUE);
			$formatted_result	= Array();
			$i	= 0;
			foreach ($schemaids as $schemaid	=> $grant) {
				$formatted_result[$i]['SchemaID']	= $schemaid;
				$formatted_result[$i]['Grant']	= $grant;
				$i++;
			}//end foreach
			return Array (
					'GetSchemasOnAssetResult'	=> $formatted_result,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID Is Not Valid. Please Provide A Valid Asset ID');
		}//end else

	}//end GetSchemasOnAsset()


	/**
	* Description: This operation will return the metadata fields ids which belong to a metadata schema
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		 'SchemaID'			=> [The metadata schema in query],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function GetMetadataFieldsOfSchema($request)
	{
		$request_info	= (Array) $request;
		$schemaid		= array_get_index($request_info, 'SchemaID', '');

		if (!empty($schemaid)) {
			$schema	= $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (!($schema instanceof Metadata_Schema)) {
				throw new SoapFault('Server', 'The schema ID provided does not belong to a metadata schema');
			}//end if
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$fieldids = $mm->getMetadataFields($schemaid);
			return Array (
					'GetMetadataFieldsOfSchemaResult'	=> array_keys($fieldids),
				   );
		} else {
			throw new SoapFault('Server', 'The schema ID provided is not valid. Please provide a valid ID.');
		}//end else

	}//end GetMetadataFieldsOfSchema()


	/**
	* Description: This operation will return the metadata fields details which belong to a metadata schema
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		 'SchemaID'			=> [The metadata schema in query],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function GetMetadataFieldsDetailOfSchema($request)
	{
		$request_info	= (Array) $request;
		$schemaid		= array_get_index($request_info, 'SchemaID', '');

		if (!empty($schemaid)) {
			$schema	= $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (!($schema instanceof Metadata_Schema)) {
				throw new SoapFault('Server', 'The schema ID provided does not belong to a metadata schema');
			}//end if
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$fieldids = $mm->getMetadataFields($schemaid);

			// Metadata field's generic attributes
			$attrs = Array(
						'name' 				=> 'Name', 
						'friendly_name' 	=> 'FriendlyName', 
						'description' 		=> 'Description', 
						'default' 			=> 'Default',
						'scheme' 			=> 'Scheme', 
						'language' 			=> 'Language', 
						'editable' 			=> 'Editable', 
						'is_contextable' 	=> 'IsContextable', 
						'required' 			=> 'Required', 
						'frontend' 			=> 'Frontend',
					);

			$fields_data = Array();
			$count = 0;
			foreach($fieldids as $fieldid => $field_info) {
				$fields_data[$count]['AssetID'] = $fieldid;
				$fields_data[$count]['TypeCode'] = $field_info[0]['type_code'];

				// Get all attribute values
				$attrs_val = $GLOBALS['SQ_SYSTEM']->am->getAllAttributeValues($fieldid, $field_info[0]['type_code']);
				foreach($attrs as $attr_name => $formatted_name) {
					$attr_val = '';
					$attr_info = array_get_index($attrs_val, $attr_name, '');
					if (isset($attr_info['value'])) {
						$attr_val = $attr_info['value'];
					}
					$fields_data[$count][$formatted_name] = $attr_val;
				}//end foreach
				$count++;
			}//end foreach

			return Array (
					'GetMetadataFieldsDetailOfSchemaResult'	=> $fields_data,
				   );
		} else {
			throw new SoapFault('Server', 'The schema ID provided is not valid. Please provide a valid ID.');
		}//end else

	}//end GetMetadataFieldsDetailOfSchema()


	/**
	* Description: This operation will return the values of the field names which belong to an asset
	*
	* @param array $request	The request information
	* <pre>
	* Array (
	*		'AssetID'			=> [The asset which has the metadata field],
	*		'FieldNames'		=> [The metadata field names],
	*        )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function GetMetadataFieldValues($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$field_names	= array_get_index($request_info, 'FieldNames', '');

		if (!empty($assetid)) {
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$schemas	= $mm->getSchemas($assetid);
			if (empty($schemas)) {
				throw new SoapFault('Server', 'There is no metadata schema applied on this asset');
			}//end if
			if(empty ($field_names)) {
				$field_names = Array();
			}
			else if (!is_array($field_names)) {
				$field_names	= Array ($field_names);
			}//end if
			$field_values = $mm->getMetadataFieldValues($assetid, $field_names);
			$field_values_final	= Array();
			$i = 0;
			foreach ($field_values as $field_name => $field_value) {
				$field_values_final[$i]['FieldName']	= $field_name;
				// bug fix #4646 Empty response returned from metadata field containing non-ascii character
				// we will need to escape the characters if we are not on a utf-8 system
				$field_values_final[$i]['FieldValue']	= $mm->escapeMetadata($field_value);
				$i++;
			}//end foreach

			return Array (
					'GetMetadataFieldValuesResult'	=> $field_values_final,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID Is Not Valid. Please Provide A Valid Asset ID');
		}//end else

	}//end GetMetadataFieldValues()


	/**
	* getComplexType
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$schema_info					= self::getArgType('SchemaSetInfo', 0, 'unbounded', TRUE);
		$field_values					= self::getArgType('FieldInfo', 0, 'unbounded', TRUE);
		$grant_type						= self::getArgType('Grant', 1, 1);
		$metadata_info					= self::getArgType('MetadataInfo', 0, 'unbounded', TRUE);
		$fields_attr_info				= self::getArgType('MetadataFieldAttrInfo', 0, 'unbounded', TRUE);

		$complex_types	= Array (
							'SetMetadataSchema'					=> Array (
																	'SetMetadataSchema'						=> Array (
																												'AssetID'								=> self::$string_non_optional,
																												'SchemaID'          					=> self::$string_non_optional,
																												'Grant'      							=> $grant_type,
																												'Cascade'								=> self::$boolean_optional,
																											   ),
																	'SetMetadataSchemaResponse'				=> Array (
																												'SetSchemaResult'						=> self::$boolean_optional,
																											   ),
																   ),
							'RegenerateMetadataSchema'			=> Array (
																	'RegenerateMetadataSchema'				=> Array (
																												'SchemaID'								=> self::$string_non_optional,
																											   ),
																	'RegenerateMetadataSchemaResponse'		=> Array (
																												'RegenerateMetadataSchemaResult'		=> self::$boolean_optional,
																											   ),
																   ),
							'RegenerateMetadataAsset'			=> Array (
																	'RegenerateMetadataAsset'				=> Array (
																												'AssetID'								=> self::$string_non_optional,
																											   ),
																	'RegenerateMetadataAssetResponse'		=> Array (
																												'RegenerateMetadataAssetResult'			=> self::$boolean_optional,
																											   ),
																   ),
							'SetAssetMetadata'					=> Array(
																	'SetAssetMetadata'						=> Array (
																												'AssetID'								=> self::$string_non_optional,
																												'FieldID'								=> self::$string_non_optional,
																												'NewValue'								=> self::$string_non_optional,
																											   ),
																	'SetAssetMetadataResponse'				=> Array (
																												'SetAssetMetadataResult'				=> self::$boolean_optional,
																											   ),
																   ),
							'SetMultipleMetadataFields'			=> Array(
																	'SetMultipleMetadataFields'				=> Array (
																												'AssetID'								=> self::$string_non_optional,
																												'MetadataInfo'							=> $metadata_info,
																											   ),
																	'SetMultipleMetadataFieldsResponse'		=> Array (
																												'SetMultipleMetadataFieldsResult'		=> self::$boolean_optional,
																											   ),
																   ),
							'SetMetadataFieldDefaultValue'		=> Array(
																	'SetMetadataFieldDefaultValue'			=> Array (
																												'FieldID'								=> self::$string_non_optional,
																												'NewDefaultValue'						=> self::$string_non_optional,
																											   ),
																	'SetMetadataFieldDefaultValueResponse'	=> Array (
																												'SetMetadataFieldDefaultValueResult'	=> self::$boolean_optional,
																											   ),
																   ),
							'GetMetadataValueByIDs'				=> Array (
																	'GetMetadataValueByIDs'					=> Array (
																												'AssetID'								=> self::$string_non_optional,
																												'FieldID'								=> self::$string_non_optional,
																											   ),
																	'GetMetadataValueByIDsResponse'			=> Array (
																												'GetMetadataValueByIDsResult'			=> self::$string_non_optional,
																											   ),
																   ),
							'GetSchemasOnAsset'					=> Array (
																	'GetSchemasOnAsset'						=> Array (
																												'AssetID'								=> self::$string_non_optional,
																											   ),
																	'GetSchemasOnAssetResponse'				=> Array (
																												'GetSchemasOnAssetResult'				=> $schema_info,
																											   ),
																   ),
							'GetMetadataFieldsOfSchema'			=> Array (
																	'GetMetadataFieldsOfSchema'				=> Array (
																												'SchemaID'								=> self::$string_non_optional,
																											   ),
																	'GetMetadataFieldsOfSchemaResponse'		=> Array (
																												'GetMetadataFieldsOfSchemaResult'		=> self::$string_optional_unbounded,
																											   ),
																   ),
							'GetMetadataFieldsDetailOfSchema'			=> Array (
																	'GetMetadataFieldsDetailOfSchema'				=> Array (
																												'SchemaID'								=> self::$string_non_optional,
																											   ),
																	'GetMetadataFieldsDetailOfSchemaResponse'		=> Array (
																												'GetMetadataFieldsDetailOfSchemaResult'		=> $fields_attr_info,
																											   ),
																   ),
							'GetMetadataFieldValues'			=> Array (
																	'GetMetadataFieldValues'				=> Array (
																												'AssetID'								=> self::$string_non_optional,
																												'FieldNames'							=> self::$string_optional_unbounded,
																											   ),
																	'GetMetadataFieldValuesResponse'		=> Array (
																												'GetMetadataFieldValuesResult'			=> $field_values,
																											   ),
																   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;


	}//end getComplexTypes();


	/**
	* getSimpleRestrictedTypes
	*
	* @return array
	* @access public
	*/
	public static function getSimpleRestrictedTypes($func_list=Array())
	{
		$simple_restricted_types	= Array (
							'SetMetadataSchema'	=> Array (
												'Grant'	=> Array (
																'restriction_base'	=> 'string',
																'enumeration'		=> Array (
																						'Apply', 'Deny', 'Revoke',
																					   ),
															   ),
											   ),
						  );

		$simple_restricted_types_available = parent::processSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


	/**
	* getComplexTypes: return all the complex types used by the API
	*
	* @return array
	* @access public
	*/
	public static function getComplexTypes($func_list=Array())
	{
			$attrs = Array(
						'name' 				=> 'Required', 
						'friendly_name' 	=> 'FriendlyName', 
						'description' 		=> 'Description', 
						'default' 			=> 'Default',
						'scheme' 			=> 'Scheme', 
						'language' 			=> 'Language', 
						'editable' 			=> 'Editable', 
						'is_contextable' 	=> 'IsContextable', 
						'required' 			=> 'Required', 
						'frontend' 			=> 'Frontend',
					);
		$complex_types    = Array (
							'GetMetadataFieldValues'    => Array (
															'FieldInfo'					=> Array (
																							'FieldName'			=> self::$string_non_optional,
																							'FieldValue'		=> self::$string_non_optional,
																						   ),
														   ),
							'GetSchemasOnAsset'			=> Array (
															'SchemaSetInfo'				=> Array (
																							'SchemaID'			=> self::$string_non_optional,
																							'Grant'				=> self::$int_non_optional,
																						   ),
														   ),
							'SetMultipleMetadataFields'	=> Array (
															'MetadataInfo'				=> Array (
																							'FieldID'			=> self::$string_non_optional,
																							'FieldValue'		=> self::$string_non_optional,
																						   ),
														   ),
							'GetMetadataFieldsDetailOfSchema' => Array(
																	'MetadataFieldAttrInfo'	=> Array(
																							'AssetID'			=> self::$string_non_optional,
																							'TypeCode'			=> self::$string_non_optional,
																							'Name'				=> self::$string_non_optional,
																							'FriendlyName'		=> self::$string_non_optional,
																							'Description'		=> self::$string_non_optional,
																							'Default'			=> self::$string_non_optional,
																							'Scheme'			=> self::$string_non_optional,
																							'Language'			=> self::$string_non_optional,
																							'Editable'			=> self::$string_non_optional,
																							'IsContextable'		=> self::$string_non_optional,
																							'Required'			=> self::$string_non_optional,
																							'Frontend'			=> self::$string_non_optional,
																							),
								),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;

	}//end getComplexTypes()

}//end class
?>
