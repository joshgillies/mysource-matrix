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
* $Id: trim_common.inc,v 1.32.2.5 2013/09/02 02:06:30 cupreti Exp $
*
*/

/*
* TRIM Package Library Functions
*
* This file contains global functions that are useful throughout the TRIM module
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @author Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.32.2.5 $
* @package MySource_Matrix_Packages
* @subpackage trim
*/


class TRIM_Common
{

	/**
	* Construct an array represent the TRIMRequest object being sent
	*
	* @return unknown
	* @access public
	*/
	public static function constructTrimRequest()
	{
		$trim_request	= Array (
							'HideVersionNumbers'		=> TRUE,
							'ProvideTimingResults'		=> FALSE,
							'ForceRealTimeCacheUpdate'	=> TRUE,
						  );

		return $trim_request;

	}//end constructTrimRequest()


	
	/**
	* Construct an xml string representing a serial of logic grouped RecordStringSearchClause
	* 
	* @param array $search_clause	The search clauses which will be converted to xml
	* @param array $classifications	The classifications uris. Search will be restricted to those uris.
	* 
	* @return string
	* @access public
	*/
	public static function constructRecordSearchClauseGroupedXml ($search_clause = Array(), $classifications)
	{
		if (!isset ($search_clause['search_groups'])) return '';
		try {
			$group_clause = new SimpleXMLElement('<RecordSearchGroups/>');
			$i = 0;
			$global_logic = isset($search_clause['global_logic']) ? $search_clause['global_logic'] : 'AND';
			foreach ($search_clause['search_groups'] as $group) {
				if (isset ($group['content'])) {
					$group_logic = isset($group['group_logic']) ? $group['group_logic'] : 'AND';
					$j = 0;
					foreach ($group['content'] as $field) {
						$field['name'] = htmlspecialchars($field['name']);
						$field['keyword'] = htmlspecialchars($field['keyword']);
						if (preg_match('/^udf:/', $field['name'])) {
							$RecordUserFieldSearchClause = $group_clause->addChild('RecordUserFieldSearchClause');
							$RecordUserFieldSearchClause->addChild('UserDefinedFieldName', $field['name']) ;
							$RecordUserFieldSearchClause->addChild('From', $field['keyword']) ;
						}
						else {
							$RecordStringSearchClause = $group_clause->addChild('RecordStringSearchClause');
							$RecordStringSearchClause->addChild('Type', $field['name']) ;
							$RecordStringSearchClause->addChild('Arg', $field['keyword']) ;
						}
						// add group logic operator
						if($j > 0) {
					    if($group_logic === 'AND') {
							$group_clause->addChild('RecordAndSearchClause');
						    }
						    else {
							$group_clause->addChild('RecordOrSearchClause');
						    }
						}
						$j++;
					}
				
				}
				if($i > 0) {
				    // add global logic operator
				    if($global_logic === 'AND') {
					    $group_clause->addChild('RecordAndSearchClause');
				    }
				    else {
					    $group_clause->addChild('RecordOrSearchClause');
				    }
				}
				$i++;
			}
		
			// add classification restrictions
			if (!empty($classifications)) {
				$i = 0;
				foreach ($classifications as $uri => $title) {
					$RecordClassificationSearchClause = $group_clause->addChild('RecordClassificationSearchClause');
					$RecordClassificationSearchClause->addChild('ClassificationUri', $uri);
					if($i > 0) {
						$group_clause->addChild('RecordOrSearchClause');
					}
					$i++;			
				}		
				if(count($search_clause['search_groups']) > 0 )
					$group_clause->addChild('RecordAndSearchClause');
			}
		
		
			// convert to string
			$result = '';
			foreach ($group_clause->children() as $child) {
				$result .= $child->asXML();
			}
		} catch (Exception $e) {
			trigger_error("Unable to contruct the XML string:".$e->getMessage(), E_USER_WARNING);
			return '';
		}

		return $result;
	}//end constructRecordSearchClauseGroupedXml()
	
	
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
	public static function constructRecordSearch($search_clauses=Array(), $record_search_options)
	{
		$recordSearch	= Array (
							'Id'								=> 'recordsearch',
							'Sort1Descending'					=> (boolean) $record_search_options['sort_1_descending'],
							'Sort2Descending'					=> (boolean) $record_search_options['sort_2_descending'],
							'Sort3Descending'					=> (boolean) $record_search_options['sort_3_descending'],
							'TargetForUpdate'					=> FALSE,
							'IsForUpdate'						=> FALSE,
							'Limit'								=> $record_search_options['limit'],
							'Skip'								=> 0,
							'Sort1'								=> $record_search_options['sort_1'],
							'Sort2'								=> $record_search_options['sort_2'],
							'Sort3'								=> $record_search_options['sort_3'],
							//'Record'.$record_search_type.'Clause'	=> $search_clause,
							'FilterRecordTypesInclude'			=> $record_search_options['record_type_filter'],
							'FilterRecordTypesExclude'			=> '',
							'FilterRecordClassInclude'			=> '',
							'FilterRecordClassExclude'			=> '',
							'FilterRecordDispositionInclude'	=> '',
							'FilterRecordDispositionExclude'	=> '',
							'FilterFileTypes' 					=> $record_search_options['file_types'],
							'FilterFinalizedState' 				=> "Both",
							'Uri' 								=> '',
							'IgnoreOnError'						=> FALSE,
						);

		$recordSearch	= array_merge($recordSearch, $search_clauses);
		return $recordSearch;

	}//end constructRecordSearch()


	/**
	* construct an array to represent the ShortcutRecordUri object in the WSDL
	*
	* @param int $uri	The Unique Resource Identifier of each record
	*
	* @return array
	* @access public
	*/
	public static function constructShortcutRecordUri($uri)
	{
		$sru	= Array (
					'Uri'				=> $uri,
					'TargetForUpdate'	=> TRUE,
					'IsForUpdate'		=> TRUE,
					'Limit'				=> 0,
					'Skip'				=>0,
				  );	

		return $sru;

	}//end constructShortcutRecordUri()


	/**
	* construct an array to represent the ShortcutRecordUris object in the WSDL
	*
	* @param array	$uris	The array containing Unique Resource Identifier of each record
	*
	* @return array
	* @access public
	*/
	public static function constructShortcutRecordUris($uris)
	{
		$srus	= Array (
					'Uris'				=> $uris,
					'TargetForUpdate'	=> FALSE,
					'IsForUpdate'		=> FALSE,
					'Limit'				=> 10,
					'Skip'				=> 0,    
				  );	
		return $srus;

	}//end constructShortcutRecordUris()


	/**
	* construct an array to represent the Download object in the WSDL
	*
	* @param string	$trans_limit	The maximum transfer limit in bytes
	*
	* @return void
	* @access public
	*/
	public static function constructDownload($download_limit='4194304')
	{
		$download	= Array (
						'Checkout'				=> FALSE,
						'MaximumTransferBytes'	=> $download_limit,
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
	public static function constructUpload($base64PayLoad)
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
	public static function constructFetch($record_properties=Array(), $search_limit)
	{
		$spec_props = Array();
		foreach ($record_properties as $key => $value) {
			$spec_props[]	= Array (
								'Name'	=> $key,
							  );
		}//end foreach

		$fetch	= Array (
							'Id'				=> 'recordfetch',
							'IsForUpdate'		=> FALSE,
							'Items'				=> $spec_props,
							'TargetForUpdate'	=> FALSE,
							'Limit'				=> $search_limit,
							'Populate'			=> 0,
							'HideVersion'		=> FALSE,
						  );
		return $fetch;

	}//end constructFetch()


	/**
	* This function is used to create new TRIM record
	*
	* @return array
	* @access public
	*/
	public static function constructCreate($container, $title, $author, $security_level, $additional_fields)
	{
		$record_type				= Array (
										'Name'	=> 'recRecordType',
										'Val'	=> 'inject:recordtype',
									  );
		$container					= Array (
										'Name'	=> 'recContainer',
										'Val'	=> $container,
									  );
		$record_title				= Array (
										'Name'	=> 'recTitle',
										'Val'	=> $title,
									  );
		$record_security_profile	= Array (
										'Name'	=> 'recSecurity',
										'Val'	=> $security_level,	// Unrestricted = 10
									  );
		$record_assignee			= Array (
										'Name'	=> 'recCurrentLoc',
										'Val'	=> $author,
									  );
		$record_author				= Array (
										'Name'	=> 'recAuthorLoc',
										'Val'	=> $author,
									  );

		$create						= Array (
										'Saving'					=> TRUE,
										'VerifyAndCreateWarning'	=> FALSE,

										'TargetForUpdate'			=> TRUE,
										'Items'						=> Array (
																		'InputProperty'	=> Array (
																								$record_type,
																								$record_title,
																								$record_assignee,
																								$container,
																								$record_security_profile,
																								$record_author,
																						   ),
																	  ),
										'TrimObjectType'			=> "record",
									  );

		foreach ($additional_fields as $field_name	=> $value) {
			$new_input_property		= Array (
										'Name'	=> $field_name,
										'Val'	=> $value,
									  );
			$create['Items']['InputProperty'][]	= $new_input_property;
		}//end if


		return $create;

	}//end constructCreate()


	/**
	* This function return the Unique Resource Identifier for a particular Record Number
	*
	* @return int
	* @access public
	*/
	public static function getURIFromRecordNumber($connection, $record_number)
	{
		$results	= Array();
		$RecordStringSearchClause	= Array (
										'RecordStringSearchClause'	=> Array (
																		'Arg'	=> $record_number,
																		'Type'	=> "RecordNumber",
																	   ),
									  );
		$record_properties			= Array ('recTitle');
		$record_search_options		= self::getDefaultRecordSearchOptions();

		$operations = Array (
						'Fetch'						=> self::constructFetch(Array(), 1),
						'RecordSearch'				=> self::constructRecordSearch($RecordStringSearchClause, $record_search_options),
					  );

		$results = self::executeRequest($connection, $operations, 'Fetch');

		if (!empty($results)) {
			if (isset($results[0]['Uri'])) {
				return $results[0]['Uri'];
			}//end
		}//end if

		return 0;

	}//end getURIFromRecordNumber()

	
	
	/**
	* This function return the Unique Resource Identifiers of a classification and all its sub-classifications
	*
	* @return array
	* @access public
	*/
	public static function getURIsFromClassification($connection, $classification)
	{
		$results	= Array();
		
		$specific_items = Array();
		$specific_items['plnTitle'] = 1;

		$operations = Array (
						'ClassificationStringSelect'	=> Array (
																		'Arg'				=>	$classification,
																		'Type'				=>	'ByWord',
																		'TargetForUpdate'	=>	FALSE,
																		'IsForUpdate'		=>	FALSE,
																		'Limit'				=>	100000,
																		'Skip'			=> 0,    
														),
						'Fetch'							=> self::constructFetch($specific_items, 100000),							
					  );

		$results = self::executeRequest($connection, $operations, 'Fetch');
	
		return $results;

	}//end getURIsFromClassification()
	
	
	/**
	* This function returns the Record Type names from the uris of Record Types
	*
	* @return array
	* @access public
	*/
	public static function getRecordTypeNamesFromUris($connection, $uris)
	{
		$results	= Array();
		
		$specific_items = Array();
		$specific_items['rtyName'] = 1;

		$operations = Array (
						'RecordTypeUrisSelect'	=> Array (
																		'Uris'				=>	$uris,
																		'BehaviorFilter'	=>	'Document',
																		'TypeFilter'		=>	'All',
																		'TargetForUpdate'	=>	FALSE,
																		'IsForUpdate'		=>	FALSE,
																		'Limit'				=>	100000,
																		'Skip'			=> 0,
														),
						'Fetch'							=> self::constructFetch($specific_items, 100000),							
					  );

		$results = self::executeRequest($connection, $operations, 'Fetch');
	
		foreach ($results as $record_info) {
			$record_names[$record_info['Uri']] = $record_info['rtyName'];
		}
		return $record_names;

	}//end getRecordTypeNamesFromUris()
	

	
	/**
	* This function returns the Record Type names from the uris of Record Types
	*
	* @return array
	* @access public
	*/
	public static function getAllRecordTypes($connection)
	{
		$results	= Array();
		
		$specific_items = Array();
		$specific_items['rtyName'] = 1;
		$behaviours = self::getRecordBehaviours();

		foreach($behaviours as $behaviour) {
			$operations = Array (
						'RecordTypeSimpleSelect'	=> Array (
																		'Type'				=>	'All',
																		'BehaviorFilter'	=>	$behaviour,
																		'TypeFilter'		=>	'All',
																		'TargetForUpdate'	=>	FALSE,
																		'IsForUpdate'		=>	FALSE,
																		'Limit'				=>	100000,
																		 'Skip'			=> 0 ,  
														),
						'Fetch'							=> self::constructFetch($specific_items, 100000),								
					  );
					

			$result = self::executeRequest($connection, $operations, 'Fetch');
			$results = array_merge($results, $result);
		}
		
		foreach ($results as $record_info) {
			$record_names[$record_info['Uri']] = $record_info['rtyName'];
		}
		return $record_names;

	}//end getAllRecordTypes()
	
	
	/**
	* This function return
	*
	* @param int	$container_uri	The URI of the container
	*
	* @return array
	* @access public
	*/
	public static function constructRecordContainerAltClause($container_uri)
	{
		$search_clause	= Array (
							'Uri'	=> $container_uri,
							'IncludeAlternateContainers'	=> FALSE,
						  );

		return $search_clause;

	}//end constructR


	/**
	* This function return all the records lie under a file/folder
	*
	* @return array
	* @access puclic
	*/
	public static function getDocumentListUnderContainer($container_uri)
	{
		$results	= Array();
		$RecordContainerAltClause	= Array (
										'RecordContainerAltClause'	=> self::constructRecordContainerAltClause($container_uri),
									  );

		$operations = Array (
							'Fetch'						=> self::constructFetch(),
							'RecordSearch'				=> self::constructRecordSearch($RecordContainerAltClause),
						  );

		$results = self::executeRequest($operations, 'Fetch');


		return $results;

	}//end getDocumentListUnderContainer()


	/**
	* this function return the record search types() available for searching
	*
	* @return array
	* @access public
	*/
	public static function getRecordSearchTypes()
	{
		$rsTypes	= Array (
						'StringSearch'			=> 'Record Content',
						'DateRangeSearch'		=> 'Record Date Range',
						'NumberRangeSearch'		=> 'Record Number Range',
					  );

		return $rsTypes;
	}//end getRecordSearchTypes()


	
	/**
	* this function return the record behaviours
	*
	* @return array
	* @access public
	*/
	public static function getRecordBehaviours()
	{
		$types	= Array (
						'Document'				=> 'Document',
						'Folder'				=> 'Folder',
						'Series'				=> 'Series',
						'Box'					=> 'Box',
						'PaperFolder'			=> 'PaperFolder',
						'Blueprint'				=> 'Blueprint',
						'Clause'				=> 'Clause',
					  );

		return $types;

	}//end getRecordBehaviours()
	
	
	/**
	* this function return the record string search clause types() available for searching
	*
	* @return array
	* @access public
	*/
	public static function getRecordStringSearchClauseType()
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
	public static function getRecordDateRangeSearchClauseType()
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
	public static function getRecordProperties()
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
						'recContainerTitle'					=> 'Container Title',
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
	* this function return the record properties available for sorting
	*
	* @return array
	* @access public
	*/
	public static function getRecordSortProperties()
	{
		$properties = Array (
						'Number'					=> 'Number',
						'FreeTitle'					=> 'FreeTitle',
						'ExternalId'				=> 'ExternalId',
						'DateRegistered'			=> 'DateRegistered',
						'DateCreated'				=> 'DateCreated',
						'DateDue'					=> 'DateDue',
						'DateClosed'				=> 'DateClosed',
						'Priority'					=> 'Priority',
						'Consignment'				=> 'Consignment',
						'DateInactive'				=> 'DateInactive',
						'DateLastAction'			=> 'DateLastAction',
						'RecordType'				=> 'RecordType',
						'StructuredTitle'			=> 'StructuredTitle',
						'DateFinalized'				=> 'DateFinalized',
						'DatePublished'				=> 'DatePublished',
					  );

		return $properties;

	}//end getRecordSortProperties()

	/**
	* This function return all the available security levels in TRIM
	*
	* @return array
	* @access public
	*/
	public static function getSecurityLevel()
	{
		$slvl	= Array (
					'Public'			=> 'Public',
					'Unrestricted'		=> 'Unrestricted',
					'In-Confidence'		=> 'In-Confidence',
					'Protected'			=> 'Protected',
					'Highly Protected'	=> 'Highly Protected',
				  );
		return $slvl;

	}//end getSecurityLevel()


	/**
	* This function return a default record search options array
	*
	* @return array
	* @access public
	*/
	public static function getDefaultRecordSearchOptions()
	{
		$record_search_options						= Array();
		$record_search_options['limit']				= '30';
		$record_search_options['file_types']		= '';
		$record_search_options['sort_1']			= 'None';
		$record_search_options['sort_2']			= 'None';
		$record_search_options['sort_3']			= 'None';
		$record_search_options['sort_1_descending']	= FALSE;
		$record_search_options['sort_2_descending']	= FALSE;
		$record_search_options['sort_3_descending']	= FALSE;
		$record_search_options['record_type_filter'] = '';

		return $record_search_options;

	}//end getDefaultRecordSearchOptions()

	/**
	* this function download the file into the specified folder
	*
	* @param array $object_list	The array contain the information about the file and its content.
	*
	* @return boolean
	* @access public
	*/
	public static function getPackagedFiles($object_list, $file_name, $final_file_path)
	{
		if (!isset($object_list['error'])) {
			$file_content	= base64_decode($object_list['Base64Payload']);
			if (empty($file_name)) {
				// Use 'AttachmentId' as default file name
				$file_name	= $object_list['AttachmentId'];
			}
			$file_name .= '.'.strtolower($object_list['FileType']);


			$file_path 		= $final_file_path.'/'.$file_name;
			if (file_exists($file_path)) {
				// Try to unlink it
				unlink($file_path);
			}//end if
			if (!$handle = fopen($file_path, 'w')) {
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
	* function to execute the web service request once all the internal objects have been set up properly
	*
	* @param array 	$operations		the operations that can be perform (search, fetch, download, upload, getaccess)
	* @param string	$display_type	the major operation that need to be achieved, so the object know how to parse the returned XML.
	* @param string $search_clause_xml the search clauses xml grouped by logical operator.
	*
	* @return array|void
	* @access public
	*/
	public static function executeRequest($connection, $operations=Array(), $display_type='Fetch', $search_clause_xml = '', $file_name='', $final_file_path='')
	{
			
			$Execute		= Array (
							'req'	=> self::constructTrimRequest(),
						 	 );
			$Execute['req']		= array_merge($Execute['req'], $operations);

			$object_list = Array();		// Result List
		
			// Get the soapclient object.
			$client = new SoapClient($connection['wsdl'], $connection['authentication']);

			// We need to convert the TRIM Execute object to XML, so we can swap the elements.

			try {
				$xml = $client->ConvertToXml($Execute);
			} catch (SoapFault $e) {
				$error_message = "Failed to ConvertToXml: ".$e->getMessage()."\n";
				if (SQ_PHP_CLI || SQ_IN_CRON) {
					$error_message .= "Last Request:\n".$client->__getLastRequest()."\n";
					$error_message .= "Last Response:\n".$client->__getLastResponse()."\n";
				}
				trigger_error($error_message, E_USER_WARNING);
				return Array();
			}


			// Now we have to swap Fetch and RecordSearch element, because RecordSearch element has to
			// come first in the soap request, otherwise TRIM wont know where the Fetch object should get the
			// result from.
			$xml = $xml->ConvertToXmlResult;
			if (isset($operations['Fetch']) && isset($operations['RecordSearch'])) {	
				$xml = self::SwapElement($xml, 'RecordSearch', 'Fetch');			
				// Add grouped search clauses xml to the converted xml
				if(!empty($search_clause_xml)) {
					$xml = preg_replace('/<\/RecordSearch>/', $search_clause_xml.'</RecordSearch>', $xml);
				}
			} else if (isset($operations['Download']) && isset($operations['ShortcutRecordUri'])) {
				$xml = self::SwapElement($xml, 'ShortcutRecordUri', 'Download');
			}
			else if (isset($operations['Fetch']) && isset($operations['ClassificationStringSelect'])) {
				$xml = self::SwapElement($xml, 'ClassificationStringSelect', 'Fetch');
			}
			else if (isset($operations['Fetch']) && isset($operations['RecordTypeUrisSelect'])) {
				$xml = self::SwapElement($xml, 'RecordTypeUrisSelect', 'Fetch');
			}
			else if (isset($operations['Fetch']) && isset($operations['RecordTypeSimpleSelect'])) {
				$xml = self::SwapElement($xml, 'RecordTypeSimpleSelect', 'Fetch');
			} else if (isset($operations['Fetch']) && isset($operations['ShortcutRecordUri'])) {
				$xml = self::SwapElement($xml, 'ShortcutRecordUri', 'Fetch');
			}

			// Now its time to get the data
			try {
				$data = $client->ExecuteXml(Array('xml'	=> $xml));
			} catch (SoapFault $e) {
				$error_message = "Failed to ExecuteXml: ".$e->getMessage()."\n";
				if (SQ_PHP_CLI || SQ_IN_CRON) {
					$error_message .= "Last Request:\n".$client->__getLastRequest()."\n";
					$error_message .= "Last Response:\n".$client->__getLastResponse()."\n";
				}
				trigger_error($error_message, E_USER_WARNING);
				return Array();
			}
			
			// The data returned is in XML format, so we have to parse it.
			$object_list = self::parseTRIMData($data->ExecuteXmlResult, $display_type);

			if (isset($operations['Fetch'])) {
				return $object_list;
			} else if (isset($operations['Download'])) {
				$res = self::getPackagedFiles($object_list, $file_name, $final_file_path);
			} else if (isset($operations['Upload'])) {
				if ($object_list['UploadId'] != $up_id) {
					echo 'Upload Successful, new UploadId: '.$object_list['UploadId'];
				}
			}//end else if


	}//end execute


	/**
	* This function find the Location URI of the a particular user based on username in TRIM.
	*
	* @param string	$assignee_name	The name of the current user who is creating the record
	*
	* @return int
	* @access public
	*/
	public static function executeGetAssigneeRequest($connection, $assignee_name)
	{
        $Execute    = Array (
                        'req'   => self::constructTrimRequest(),
                      );

		// BEGIN GET ASSIGNEE URI
        $LocationStringSelect			 = Array (
											'Arg'					=> $assignee_name,
											'HideActiveLocations'	=> FALSE,
											'HideInactiveLocations'	=> FALSE,
											'HideInternalLocations'	=> FALSE,
											'HideExternalLocations'	=> FALSE,
											'HideAdhocLocations'	=> FALSE,
											'TargetForUpdate'		=> FALSE,
											'IsForUpdate'			=> FALSE,
											'Limit'					=> 1,
											'Skip'					=> 0,   
											'Reserved'				=> FALSE,
										   );
		// END GET ASSIGNEE URI
		$FetchInjectionUriLocationId	= Array (
											'Id'					=> 'LocationId',
											'TargetForUpdate'		=> FALSE,
										  );

		$Execute['req']['FetchInjectionUri']		= $FetchInjectionUriLocationId;
		$Execute['req']['LocationStringSelect']		= $LocationStringSelect;

        $client = new SoapClient($connection['wsdl'], $connection['authentication']);

		try {
			$xml = $client->ConvertToXml($Execute);
		} catch (SoapFault $e) {
			$error_message = "Failed to ConvertToXml: ".$e->getMessage()."\n";
			if (SQ_PHP_CLI || SQ_IN_CRON) {
				$error_message .= "Last Request:\n".$client->__getLastRequest()."\n";
				$error_message .= "Last Response:\n".$client->__getLastResponse()."\n";
			}
			trigger_error($error_message, E_USER_WARNING);
			return 0;
		}
        $xml = self::SwapElement($xml->ConvertToXmlResult, 'LocationStringSelect', 'FetchInjectionUri');
 
		try {
			$data = $client->ExecuteXml(Array('xml' => $xml));
		} catch (SoapFault $e) {
			$error_message = "Failed to ExecuteXml: ".$e->getMessage()."\n";
			if (SQ_PHP_CLI || SQ_IN_CRON) {
				$error_message .= "Last Request:\n".$client->__getLastRequest()."\n";
				$error_message .= "Last Response:\n".$client->__getLastResponse()."\n";
			}
			trigger_error($error_message, E_USER_WARNING);
			return 0;
		}
		
		try {
	        $return_xml = new SimpleXMLElement((string) $data->ExecuteXmlResult);
		} catch (Exception $e){
			trigger_error("Unable to parse the XML result: ".$e->getMessage(), E_USER_WARNING);
			return 0;
		}
        $trim_userid = 0;
        if ((string) $return_xml->SearchResult->FoundCount > 0) {
        	$trim_userid = (string) $return_xml->FetchInjectionUriResult->Uri;
        }//end if

        return $trim_userid;

	}//end executeGetAssigneeRequest()
	
	
	/**
	* This function create a record in TRIM.
	*
	* @param string	$username	The username of the user who is creating the TRIM document
    * @param array  &$link  	information used to create the initial link<br/>
    * <pre>
    * Array (
    *        'record_title'		=> [The title of the new record]
    *        'record_type'   	=> [some kind of record type (document)],
    *        'parent_container'	=> [15331],
    * 		 'securiry_level'	=> [The security level of the new document],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	public static function executeCreateRequest($connection, $username, $new_record_info)
	{
		// Check Whether the username exists in TRIM
		$trim_userid = self::executeGetAssigneeRequest($connection, $username);
		
		if (!$trim_userid) {
			trigger_error('There is no such user with username "'.$username.'" in TRIM. Record creation aborted.', E_USER_WARNING);
			return FALSE;
		}//end if


		// BEGIN GET RECORD TYPE "document" URI
		$RecordTypeStringSelect			= Array (
											'Arg'   			=> $new_record_info['record_type'],
											'Id'    			=> 'recordtype',
											'TargetForUpdate'	=> FALSE,
											'IsForUpdate'		=> FALSE,
											'Limit'				=> 10,
											'Skip'				=> 0,    
											'BehaviorFilter'	=> ucwords($new_record_info['record_type']),
											'TypeFilter'		=> 'All',
										  );

		$FetchInjectionUriRecordType	= Array (
											'Id'				=> 'recordtype',
											'TargetForUpdate'	=> FALSE,
										  );
		// END GET RECORD TYPE "document" URI

		$parent_record_uri	= self::getURIFromRecordNumber($connection, $new_record_info['parent_record_number']);
		
		if (!$parent_record_uri) {
			trigger_error('There is no URI correspond to record number "'.$new_record_info['parent_record_number'].'" in TRIM. Record creation aborted.', E_USER_WARNING);
			return FALSE;
		}//end if

		$operations		= Array (
							'Create'	=> self::constructCreate($parent_record_uri, $new_record_info['record_title'], $trim_userid, $new_record_info['security_level'], $new_record_info['additional_fields']),
						  );

		$data	= NULL;
		try {
			$Execute	= Array (
							'req'		=> self::constructTrimRequest(),
						  );

			$Execute['req']								= array_merge($Execute['req'], $operations);
			$Execute['req']['RecordTypeStringSelect']	= $RecordTypeStringSelect;
			$Execute['req']['FetchInjectionUri']		= $FetchInjectionUriRecordType;

			$client = new SoapClient($connection['wsdl'], $connection['authentication']);

			$xml = $client->ConvertToXml($Execute);
			$xml = self::SwapElement($xml->ConvertToXmlResult, 'RecordTypeStringSelect', 'Create');
			$xml = self::SwapElement($xml, 'FetchInjectionUri', 'Create');

			$data = $client->ExecuteXml(Array('xml'	=> $xml));


		} catch (SoapFault $e) {
			$error_message = "Create Request Failed: ".$e->getMessage()."\n";
			if (SQ_PHP_CLI || SQ_IN_CRON) {
				$error_message .= "Last Request:\n".$client->__getLastRequest()."\n";
				$error_message .= "Last Response:\n".$client->__getLastResponse()."\n";
			}
			trigger_error($error_message, E_USER_WARNING);
			return 0;
		}//end try catch

		if(isset($data->error) && !empty ($data->error)) {
		    	trigger_error($data->error, E_USER_WARNING);
			return 0;
		}

		$new_record_uri	= 0;
		if (!empty($data)) {
			try {
				$return_xml	= new SimpleXMLElement((string) $data->ExecuteXmlResult);
			} catch (Exception $e){
				trigger_error("Unable to parse the XML result: ".$e->getMessage(), E_USER_WARNING);
				return 0;
			}
			if (isset($return_xml->ErrorResult)) {
				// We have an error, the submission has not been successful
				trigger_error('Unable to create new record. '.(string) $return_xml->ErrorResult->Message.')');
			} else {
				$new_record_uri = (string) $return_xml->FetchResult->Objects->TrimObject->attributes()->Uri;
			}//end else
		}//end if

		return $new_record_uri;

	}//end executeCreateRequest


	/**
	* this function send a header to let File be streamed via browser
	*
	* @param array $object_list	The array contain the information about the file and its content.
	*
	* @return void
	* @access public
	*/
	private static function downloadFile($object_list)
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
	* swap the two XML element that are adjacent to each other if ele_1 occurs after to ele_2
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
	public static function SwapElement($xml, $ele_1, $ele_2)
	{
		// if element 1 comes prior to element 2, there is no need to swap
		if(strpos($xml, '<'.$ele_1) < strpos($xml, '<'.$ele_2)) return $xml;
		
		$match_1	= Array();
		preg_match('/\<'.$ele_1.'\>([^"]*)\<\/'.$ele_1.'\>/', $xml, $match_1);
		$part_1 = $match_1[0];

		$match_2	= Array();
		preg_match('/\<'.$ele_2.'\>([^"]*)\<\/'.$ele_2.'\>/', $xml, $match_2);
		$part_2 = $match_2[0];

		$xml	= preg_replace('/\<'.$ele_1.'\>([^"]*)\<\/'.$ele_1.'\>/', 'PART_2', $xml);
		$xml	= preg_replace('/\<'.$ele_2.'\>([^"]*)\<\/'.$ele_2.'\>/', 'PART_1', $xml);

		$xml	= str_replace('PART_2', $part_2, $xml);
		$xml	= str_replace('PART_1', $part_1, $xml);

		return $xml;

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
	private static function parseTRIMData($data, $operation='Fetch')
	{
		$object_list = Array();
		try {
			$xml = new SimpleXMLElement($data);
		} catch (Exception $e){
			trigger_error("Unable to parse the XML result: ".$e->getMessage(), E_USER_WARNING);
			return $object_list;
		}

		if ($operation == 'Fetch') {
			if($xml->SearchResult->FoundCount ==  0) return $object_list;
			$i = 0;
			foreach ($xml->FetchResult->Objects->TrimObject as $object) {
				$uri = NULL;
				$version = NULL;
				foreach ($object->attributes() as $name => $attr_value) {
					if ((string)$name == 'Uri') $uri = (string)$attr_value;
					else if ($name = 'Version') {
						$version = (string) $attr_value;
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
	* This function test whether a connection is valid
	*
	* @return boolean
	* @access public
	*/
	public static function test_trim_connection($connection)
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
			$details  = fetch_url($connection['wsdl'], $options, array(), FALSE);
			$contents = $details['response'];
			if (strpos($contents, '?xml') === FALSE) return FALSE;
		}
		
		//check if recTitle property is set
		try {
			$client = new SoapClient($connection['wsdl'], $connection['authentication']);

			$data = $client->ListProperty(Array('PropertyName'	=> 'recTitle'));

	        $name = (string) $data->ListPropertyResult->Name;

	        if ($name == 'recTitle') {
	        	return TRUE;
	        } else {
	        	return FALSE;
	        }//end else
		} catch (SoapFault $e) {
			echo $e->getMessage();
			return FALSE;
		}//end if

	}//end test_trim_connection()
	
	/**
	* This function return specific properties of a record based on URI
	*
	* @param array $connection - details
	* @param string $uri - uri of desired subject
	* @param array $record_properties - array of return properties
	*
	* @return array
	* @access public
	*/
	public static function getRecordPropertiesFromUri($connection, $uri, $record_properties) 
	{
		$operations = Array (
								'Fetch'						=> self::constructFetch($record_properties, 1),
								'ShortcutRecordUri'			=> self::constructShortcutRecordUri($uri),
							  );
		$object_list = self::executeRequest($connection, $operations, 'Fetch');
		
		return $object_list;
	}

	
	/**
	* This function returns the location title from uris
	*
	* @return array
	* @access public
	*/
	public static function getLocationTitleFromUris($connection, $uris)
	{
		$results	= Array();
		$loc_names = Array();
		$specific_items = Array();
		$specific_items['locFullFormattedName'] = 1;

		$operations = Array (
						'LocationUrisSelect'	=> Array (
																		'Uris'				=>	$uris,
																		'IsForUpdate'		=>	FALSE,
																		'TargetForUpdate'	=>	FALSE,
																		'Limit'			=>	100000,
																		'Skip'			=> 0,    
																		'HideActiveLocations'	=>	FALSE,
																		'HideInactiveLocations'	=>	FALSE,
																		'HideInternalLocations'	=>	FALSE,
																		'HideExternalLocations'	=>	FALSE,
																		'HideAdhocLocations'	=>	FALSE,
																		'Reserved'				=>  FALSE,

														),
						'Fetch'							=> self::constructFetch($specific_items, 100000),							
					  );

		$results = self::executeRequest($connection, $operations, 'Fetch');
	
		foreach ($results as $record_info) {
			$loc_names[$record_info['Uri']] = $record_info['locFullFormattedName'];
		}

		return $loc_names;

	}//end getLocationTitleFromUris()
	
	
}//end class
