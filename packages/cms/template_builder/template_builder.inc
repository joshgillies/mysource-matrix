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
* $Id: template_builder.inc,v 1.1 2004/11/02 00:04:54 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Template_Builder
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Template_Builder extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Template_Builder($assetid=0)
	{
		$this->Page($assetid);

	}//end Template_Builder()

	
	function create(&$link)
	{
		if (!parent::create($link)) return false;
		$this->createBodycopy('Template Bodycopy', 'template');
		return true;
		
	}//end create()

	
	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks() {

		return Array(SQ_LINK_TYPE_2 => Array('bodycopy' => Array('card' => 'M', 'exclusive' => false)));

	}//end _getAllowedLinks()
	
	
	/**
	* Creates a bodycopy linked underneath this asset with specified name, value and text
	*
	* @param string	$name		the name of the bodycopy asset
	* @param string $link_value	the value attached to the link
	* @param string $bc_text	the bodycopy text to be placed inside the first
	*							bodycopy DIV
	*
	* @return boolean	true indicates success
	* @access public
	*/
	function createBodycopy($name, $link_value, $bc_text='')
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bc_asset = new Bodycopy();
		
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => $link_value, 'sort_order' => 1);
		$bc_asset->setAttrValue('name', $name);
		$attributes = Array();
		
		if (!$bc_asset->create($copy_link, $attributes)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}
		
		// not setting any bodycopy text?
		if ($bc_text == '') {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}
		
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($bc_asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);
		$link =& reset($links);
		
		$container_assetid = $link['minorid'];
		$container =& $GLOBALS['SQ_SYSTEM']->am->getAsset($container_assetid);
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($container_assetid, SQ_LINK_TYPE_2, 'content_type', false);
		$link =& reset($links);
		
		$ct_assetid = $link['minorid'];
		$content_type =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ct_assetid);
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($content_type->id, 'attributes', $content_type->id);
		$content_type->setAttrValue('html', $bc_text);
		$content_type->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($content_type->id, 'attributes');
		$cont_edit_fns =& $container->getEditFns();
		$cont_edit_fns->generateContentFile($container);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		
		return true;
	
	}//end createBodycopy()

	
	/**
	* Returns information about the question fields currently in the template bodycopy
	*
	* Format of returned array is:
	* <PRE>
	* Array('raw'    => string (actual replacement code, including hashes etc)
	*		'name'   => string (name of field),
	*		'type'	 => string (currently type of bracketed expression - '[' or '{'),
	*		'values' => Array  (exploded list of possible values)
	* </PRE>
	*
	* @return Array
	* @access public
	*/
	function &getQuestionFields()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'major', 'template');
		$link =& reset($links);
	
		$bc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
		
		$bc_text = $bc_asset->getRawBodycopyContent();
	
		preg_match_all('/#(.*)(((\[)(.*)\]|(\{)(.*)\}|(\()(.*)\)))?#/U', $bc_text, $matches, PREG_SET_ORDER);

		$fields = Array();
		
		// return of preg_match_all:
		// [1] = name of field
		// [4] = type of bracket
		// [5] = contents of the bracket
		foreach($matches as $bc_fields) {
			switch(count($bc_fields)) {
				// everything specified - freeform text, size specified. For some reason this
				// returns 10 back-references instead of 6... oh well
				case 10:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[8],
										'values' => explode('x', $bc_fields[9]),
										'raw'    => $bc_fields[0],
										'is_select' => false,
									);
				break;
				
				// everything specified - multiple selection field
				case 8:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[6],
										'values' => explode('|', $bc_fields[7]),
										'raw'    => $bc_fields[0],
										'is_select' => true,
									);
				break;
				
				// everything specified - selection field
				case 6:
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[4],
										'values' => explode('|', $bc_fields[5]),
										'raw'    => $bc_fields[0],
										'is_select' => true,
									);
				break;
				
				case 5:			// bracket specified, no contents!!
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => $bc_fields[4],
										'values' => Array(),
										'raw'    => $bc_fields[0],
										'is_select' => true,
									);
				break;
				
				default:		// no bracket specified at all = freeform text
					$fields[] = Array(	'name'   => $bc_fields[1],
										'type'	 => '',
										'values' => Array(),
										'raw'    => $bc_fields[0],
										'is_select' => false,
									);
				break;
				
			}//end switch
			
		}
		
		return $fields;
	
	}//end getQuestionFields()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	* @return void
	*/
	function printBody()
	{
		if (!isset($_POST['SQ_TEMPLATE_COMMIT'])) {
			print $this->paintDefaultForm();
		} else {
			$reps = $this->setReplacements();
			$this->createBodycopy(date('d M Y H:i:s'), '', $reps);
		}
	
	}//end printBody()
	
	
	function paintDefaultForm()
	{
		$q_fields =& $this->getQuestionFields();
		$prefix = $this->type().'_'.$this->id;
		
		$output = '<form action="'.$_SERVER['PHP_SELF'].'" method="post"><table>';

		foreach($q_fields as $q_field) {

			$field_name = $prefix.'_'.strtolower(str_replace(' ','_',$q_field['name']));
			
			$output .= '<tr><td>';
			$output .= $q_field['name'];
			$output .='</td><td>';
			
			switch ($q_field['type']) {
				// single select - drop down OR radio buttons if too long
				case '[':
					$max_length = 0;
					foreach ($q_field['values'] as $q_value) {
						$max_length = max($max_length, strlen($q_value));
					}
					
					ob_start();
					
					if ($max_length > 39) {
						foreach($q_field['values'] as $key => $q_value) {				
							radio_button($field_name, $key, false);
							echo $q_value.'<br/>';
						}
					} else {
						combo_box($field_name, $q_field['values'], false, Array());
					}
					$output .= ob_get_contents();
					ob_end_clean();
				break;
				
				// multiple select, check boxes always
				case '{':
					ob_start();
					foreach($q_field['values'] as $key => $q_value) {				
						check_box($field_name.'[]', $key, false);
						echo $q_value.'<br/>';
					}
					$output .= ob_get_contents();
					ob_end_clean();
				break;
				
				// freeform text - size specified
				case '(':
					ob_start();
					if ($q_field['values'][1] == 1) {
						text_box($field_name, '', $q_field['values'][0]);
					} else {
						text_area($field_name, '', $q_field['values'][0], $q_field['values'][1]);
					}
					$output .= ob_get_contents();
					ob_end_clean();
				break;

				// freeform text - default size of 40 columns x single row
				case '':
					ob_start();
					text_box($field_name, '', 40);
					$output .= ob_get_contents();
					ob_end_clean();
				break;
				
				default:
					$output .= '* Unknown *';
				break;
				
			}//end switch
											
			
			$output .= '</td></tr>';
		}

		$output .= '<tr><td>';
		$output .= '&nbsp;</td><td>';
		$output .= '<input type="hidden" name="SQ_TEMPLATE_COMMIT" value="true">';
		$output .= '<input type="submit" value="Submit">';
		$output .= '<input type="reset" value="Clear">';
		$output .= '</td></tr>';
		$output .= '</table></form>';
		
		return $output;
	
	}//end paintDefaultForm()
	
	
	/**
	* Throws keyword replacements into the template bodycopy and returns the contents
	*
	* @return string
	* @access public
	*/
	function setReplacements()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'major', 'template');
		$link =& reset($links);
	
		$bc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
		
		$bc_text = $bc_asset->getRawBodycopyContent();

		$q_fields = $this->getQuestionFields();
		$prefix = $this->type().'_'.$this->id;
				
		foreach($q_fields as $q_field) {
			$rep = $q_field['raw'];
			$field_name = $prefix.'_'.strtolower(str_replace(' ','_',$q_field['name']));
			
			if ($q_field['is_select']) {
				$field_index = array_get_index($_POST, $field_name, null);
				
				if (!is_null($field_index)) {
				
					if (is_array($field_index)) {
						if (count($field_index) == 0) {
						} else if (count($field_index) == 1) {
							// multiple selection field with only one selection
							$field_value = $q_field['values'][$field_index[0]];
						} else {
							// multiple selection field with more than one selection
							$field_values = Array();
							foreach ($field_index as $key => $value) {
								$field_values[] = $q_field['values'][$value];
							}
							
							$field_value = implode(', ', array_slice($field_values,0,-1));
							// $value still contains the last known value within the foreach
							$field_value .= ' and '.$q_field['values'][$value];
						}
					} else {
						// single selection field
						$field_value = $q_field['values'][array_get_index($_POST, $field_name, '')];
					}
				
				} else {
					$field_value = '';
				}
				
			} else {
				// freeform field
				$field_value = nl2br(htmlspecialchars(array_get_index($_POST, $field_name, '')));
			}
			
			$bc_text = str_replace($rep, $field_value, $bc_text);
		}
		
		return $bc_text;
		
	}
	
	
}//end class

?>
