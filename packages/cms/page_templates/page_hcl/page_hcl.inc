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
* $Id: page_hcl.inc,v 1.1 2004/01/13 05:50:49 diarandine Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page.inc';
require_once 'XML/Tree.php';

/**
* Page_HCL
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_HCL extends Page
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*/
	function Page_HCL($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/** 
	* Creates XML formatted list of the metadata entries. 
	*
	* @param int	$rootid		asset ID of the root element to start processing with
	* @param Array	$schemas	array of schema IDs, that should be used for fields selection
	*
	* @return void
	* @access private
	*/
	function createAndOutputHCL($rootid, $schemas)
	{
		// instantiating our managers
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager(); 
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		// preparing XML Tree
		$output = new XML_Tree();
		$root = &$output->addRoot('tmp', '');
	
		$str = '';

		// starting to fill in the string with XML_Tree nodes. We will not have here any <xml> tags etc.
		// because this will be directly written to the stdout right after the page header
		$record_attrs = Array ('xmlns'		=> 'http://www.openarchives.org/OAI/1.1/OAI_GetRecord',
							   'xmlns:xsi'	=> 'http://www.w3.org/2001/XMLSchema-instance',
							   'xsi:schemaLocation'	=> 'http://www.openarchives.org/OAI/1.1/OAI_GetRecord.xsd'
							  );	
		$getrecord_node = &$root->addChild('GetRecord', '', $record_attrs);

		// taking current date to fill the 'responseDate' node
		$date = date('Y-m-d\TG:i:s');
		$offset = date('O');
		$date .= substr($offset, 0, 3).':'.substr($offset, 3, 2);

		$response_node  = &$getrecord_node->addChild('responseDate', $date);
		$request_node   = &$getrecord_node->addChild('requestURL'); 
		
		// processing starts: getting all the assets under root element
		$root_kids = $am->getChildren($rootid, 'page_standard', false); 

		// looping through obtained children IDs and processing every single asset (pages)
		foreach ($root_kids as $asset_id) {
			
			// work with this asset only if it allows metadata (i.e. is not a bodycopy, etc.)
			if ($mm->allowsMetadata($asset_id)) {

				$record_node = &$getrecord_node->addChild('record'); 

				// inserting header element right after the <record>. 
				$header_node = &$record_node->addChild('header');

				// inserting two more nodes into the header node
				$asset = &$am->getAsset($asset_id); 
				$identifier = $asset->getURL();
				$datestamp = date('Y-m-d', $asset->created);
	
				$header_node->addChild('identifier', $identifier);
				$header_node->addChild('datestamp', $datestamp); // format should be '2004-01-06'

				// adding metadata element to XML_Tree
				$metadata_node = &$record_node->addChild('metadata');

				// getting the array of all metadata schemas applied to current examined asset...
				$current_schemas = $mm->getSchemas($asset_id, true);

				// looping through supplied schema IDs...
				foreach ($schemas as $schema_id) {
				
					// now checking if current schema ID actually is applied to this particular asset
					if (in_array($schema_id, $current_schemas)) {

						// If yes, then we can process the fields for 
						// this schema id for current asset:
						
						// 1). Getting the name of this schema and adding another XML node with this name
						$typecodes      = Array('metadata_schema');
						$tempid_arr		= Array(); 
						array_push($tempid_arr, $schema_id);

						$schema_attribs = $am->getAssetInfo($tempid_arr, $typecodes); 
						foreach ($schema_attribs as $key => $values_arr) {
							$tempname = $values_arr['name'];
						}
						$tempname = strtolower($tempname); // making the string lowercase
						$schema_node = &$metadata_node->addChild($tempname); // adding node...
	
						// 2). Get all the IDs of schema fields and then the array of their attribs
						$schema_fields = $mm->getMetadataFields($schema_id); 
						$type_codes	   = Array('metadata_field');
						$schema_field_attribs = $am->getAssetInfo($schema_fields, $type_codes, true); 
						
						// 3). Filling the new array with the names of the fields
						$tempname_arr = Array();
						foreach ($schema_field_attribs as $key => $field_attrs) {
							array_push($tempname_arr, $field_attrs['name']);
						}

						// 4). Getting key-value pairs for these field names
						$field_values = Array();
						$field_values = $mm->getMetadataFieldValues($asset_id, $tempname_arr);

						// 5). Looping through obtained field value pairs
						foreach ($field_values as $field_key => $field_val) {
							
							// 5.1). Before displaying the field name ('field_key'), we need
							//       to cut off the first part until the first separating dot
							$dotpos = (strpos($field_key, '.')) + 1; 
							$field_key = substr_replace($field_key, '', 0, $dotpos);

							// 6). Adding the new field XML node
							$schema_node->addChild($field_key, $field_val);
						
						} // end foreach field name/value

					} // end if in_array

				} // end foreach schemas

				$am->forgetAsset($asset);

			} // end if allows metadata

		} // end foreach

		$str .= $getrecord_node->get(); // assembling the output string from the top XML node

		// writing the XML to the screen
		header("Content-type: text/xml");
		echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
		echo $str; // our contents...

	}//end createAndOutputHCL()


	/** 
	* Creates RDF formatted list of the metadata entries. 
	*
	* @param int	 $rootid	asset ID of the root element to start processing with
	* @param Array	 $schemas	array of schema IDs, that should be used for fields selection
	*
	* @return void
	* @access private
	*/
	function createAndOutputRDF($rootid, $schemas)
	{
		// instantiating our managers
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager(); 
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		$output = new XML_Tree();

		// REGISTERING XML NAMESPACES for the future reference...
		$output->registerName('rdf', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
		$output->registerName('agls', 'http://www.naa.gov.au/recordkeeping/gov_online/agls/1.2');
		$output->registerName('dc', 'http://purl.org/dc/elements/1.1/');
		$output->registerName('edna', 'http://www.edna.edu.au/metadata/v1.1');

		// adding root element
		$root = &$output->addRoot('tmp', '');
		
		$str = '';

		// init empty namespace definitions array
		$rdf_attrs = Array();

		// first element of this array is constant, unchanging
		$rdf_attrs['xmlns:rdf'] = 'http://www.w3.org/1999/02/22-rdf-syntax-ns#';

		// looping through the array of schemas and adding their 'namespace' => 'namespace URI' to attrs
		foreach ($schemas as $schema_id) {
				
				$schema_asset = &$am->getAsset($schema_id); 
				$namespace    = $schema_asset->attr('namespace');
				$nsurl	      = $schema_asset->attr('nsurl');
				
				if (isset($namespace) && isset($nsurl)) {
					$rdf_attrs[$namespace] = $nsurl;
				}

				$am->forgetAsset($schema_asset);
		}

		// adding the RDF root node with defined set of attributes
		$rdf_root_node = &$root->addChild('rdf:RDF', '', $rdf_attrs);

		// PROCESSING STARTS HERE: getting all the assets under root element
		$root_kids = $am->getChildren($rootid, 'page_standard', false); // leave type_code empty ??

		// looping through obtained children IDs and processing every single asset (pages)
		foreach ($root_kids as $asset_id) {
			
			// work with this asset only if it allows metadata (i.e. is not a bodycopy, etc.)
			if ($mm->allowsMetadata($asset_id)) {
				
				$asset = &$am->getAsset($asset_id); // getting the asset object
				$desc_node_attrs  = Array ('about' => $asset->getURL()); 
				$description_node = &$rdf_root_node->addChild('rdf:Description', '', $desc_node_attrs); 

				// getting the array of all metadata schemas applied to current examined asset...
				$current_schemas = $mm->getSchemas($asset_id, true);

				// looping through supplied schema IDs...
				foreach ($schemas as $schema_id) {
				
					// now checking if current schema ID actually is applied to this particular asset
					if (in_array($schema_id, $current_schemas)) {

						// If yes, then we can process the fields for 
						// this schema id for current asset:
						
						// 1). Getting the name of this schema and storing it to use later as namespace pointer
						$typecodes      = Array('metadata_schema');
						$tempid_arr		= Array(); 
						array_push($tempid_arr, $schema_id);

						$schema_attribs = $am->getAssetInfo($tempid_arr, $typecodes); 
						foreach ($schema_attribs as $key => $values_arr) {
							$tempname = $values_arr['name'];
						}
						$namespace = strtolower($tempname); 

						// 2). Get all the IDs of schema fields and then the array of their attribs
						$schema_fields = $mm->getMetadataFields($schema_id); 
						$type_codes	   = Array('metadata_field');
						$schema_field_attribs = $am->getAssetInfo($schema_fields, $type_codes, true); 
						
						// 3). Filling the new array with the names of the fields
						$tempname_arr = Array();
						foreach ($schema_field_attribs as $key => $field_attrs) {
							array_push($tempname_arr, $field_attrs['name']);
						}

						// 4). Getting key-value pairs for these field names
						$field_values = Array();
						$field_values = $mm->getMetadataFieldValues($asset_id, $tempname_arr);

						// 5). Looping through obtained field value pairs
						foreach ($field_values as $field_key => $field_val) {
							
							// 5.1). Before displaying the field name ('field_key'), we need
							//       to cut off the first part until the first separating dot
							$dotpos = (strpos($field_key, '.')) + 1; 
							$field_key = substr_replace($field_key, '', 0, $dotpos);

							// 5.2). Adding the namespace to the field key (name)
							$field_key = $namespace . ':' . $field_key;

							// 6). Adding the new field XML node with the namespace
							$description_node->addChild($field_key, $field_val);
						
						} // end foreach field name/value

					} // end if in_array

				} // end foreach schemas

				$am->forgetAsset($asset);

			} // end if allows metadata

		} // end foreach

		$str .= $rdf_root_node->get(); // assembling the output string from the top XML node

		// writing the XML to the screen
		header("Content-type: text/xml");
		echo "<?xml version=\"1.0\" encoding=\"iso-8859-1\"  standalone=\"yes\" ?>\n";
		echo "<!DOCTYPE rdf:RDF [\n".
			 "<!ENTITY rdfns 'http://www.w3.org/1999/02/22-rdf-syntax-ns#'>\n".
			 "<!ENTITY rdfsns 'http://www.w3.org/2000/01/rdf-schema#'>\n".
			 "<!ENTITY dcns 'http://purl.org/dc/elements/1.1/'>\n".
			 "<!ENTITY dctermsns 'http://purl.org/dc/terms/'>\n".
			 "<!ENTITY dctypens 'http://purl.org/dc/dcmitype/'>\n".
			 "]>\n";
		echo $str; // our contents...

	}//end createAndOutputRDF()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		$schemas = $this->attr('schemas');
		$rootid	 = $this->attr('rootid'); 
		$type	 = $this->attr('type');
	
		if ($type == 1) {
			$testresult = $this->createAndOutputRDF($rootid, $schemas);
		} else {
			$testresult = $this->createAndOutputHCL($rootid, $schemas);
		}

	}//end printFrontend()

}//end class
?>