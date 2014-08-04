<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_section.inc,v 1.89 2013/08/22 00:27:08 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* Form_Section
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Revision: 1.89 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section extends Asset implements Bridge
{

	/**
	* the question index type
	* @var integer
	*/
	var $question_index = 0;

	/**
	* the width of the question name column (used when printing a generic section)
	* @var integer
	*/
	var $question_col_width = 0;

	/**
	* the width of the answer column (used when printing a generic section)
	* @var integer
	*/
	var $answer_col_width = 0;

	/**
	* the active submission array
	* @var Array
	*/
	var $current_answers = Array();


	/**
	* the active submission extra-data array
	* @var Array
	*/
	var $extra_data = Array();


	/**
	* the active submission array
	* @var Array
	*/
	var $submission_errors = Array();

	/**
	* parent form/section id
	* @var Array
	*/
	var $parent_form_id = null;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Section($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


//--        GENERIC ASSET FUNCTIONS        --//


	/**
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=TRUE)
	{
		if (!parent::_updated($update_parents)) return FALSE;
		// do not regenerate file during clone, do it afterwards
		$in_clone = isset($GLOBALS['SQ_CLONE_COMPONENTS']) && $GLOBALS['SQ_CLONE_COMPONENTS'];
		if (!$in_clone) return $this->fileRegeneration();

		return TRUE;

	}//end _updated()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
					SQ_LINK_TYPE_2	=> Array(
										'form_section'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
														   ),
										'bodycopy'		=> Array(
															'card'		=> '1',
															'exclusive'	=> FALSE,
														   ),
									   ),
					SQ_LINK_TYPE_3	=> Array(
										'bodycopy'		=> Array(
															'card'		=> '1',
															'exclusive'	=> FALSE,
														   ),
									   ),
			   );

	}//end _getAllowedLinks()



	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a bodycopy container then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Form_Section) && $dependant != '1') {
			$dependant = '1';
			$link_type = SQ_LINK_TYPE_2;
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()



	/* Clones certain specified components of the asset
	*
	* @param object		&$clone		the clone shell
	* @param array		$components	the wanted components to clone eg.
	* <pre>
	* 								Array(
	* 									'attributes',
	* 									'metadata_schemas',
	* 									'metadata',
	* 									'workflow',
	* 									'permissions',
	* 									'data',
	* 									'content_tags',
	* 									'roles',
	* 								);
	* 								or alternately
	* 								Array('all');
	* </pre>
	* @param boolean	$override	whether or not to override the existing permission, metadata schemas, workflow schemas with the new ones.
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components, $override=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::cloneComponents($clone, $components, $override)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_CLONE_COMPONENTS'] = TRUE;

		if (in_array('attributes', $components) || in_array('all', $components)) {
			$sort_order = $this->attr('sort_order');
			$sort_order = str_replace($this->id.':q', $clone->id.':q', $sort_order);

			// save the information
			$clone->setAttrValue('sort_order', $sort_order);
			$clone->saveAttributes();

			// Now remap any parent forms, but only if we are being cloned along with a form clone:
			// remap to the cloned form. If we are being clone within a form, we will not remap.
			$form_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id, 'form_email', FALSE);
			$clone_form_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($clone->id, 'form_email', FALSE);

			$clone_form_parents = array_diff($clone_form_parents, $form_parents);

			foreach ($clone_form_parents as $parent_assetid) {
				$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_assetid);

				$recip = unserialize($form->attr('recipient_emails_format'));
				$recip = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $recip);
				$rec = unserialize($form->attr('receipt_emails_format'));
				$rec = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $rec);
				$staf = unserialize($form->attr('staf_format'));
				$staf = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $staf);
				$sel = $form->attr('selective_emails');
				foreach ($sel as $key => $null) {
					$sel[$key]['address'] = str_replace('%response_'.$this->id.'_', '%response_'.$clone->id.'_', $sel[$key]['address']);
					$sel[$key]['assetid'] = str_replace($this->id.':q', $clone->id.':q', $sel[$key]['assetid']);
				}

				// field selections for STAF and receipt
				$staf_field = str_replace($this->id.':q', $clone->id.':q', $form->attr('staf_field'));
				$receipt_field = str_replace($this->id.':q', $clone->id.':q', $form->attr('receipt_field'));

				// save the information
				$form->setAttrValue('recipient_emails_format', serialize($recip));
				$form->setAttrValue('receipt_emails_format', serialize($rec));
				$form->setAttrValue('staf_format', serialize($staf));
				$form->setAttrValue('selective_emails', $sel);

				$form->setAttrValue('staf_field', $staf_field);
				$form->setAttrValue('receipt_field', $receipt_field);

				$form->saveAttributes();
			}
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
		return TRUE;

	}//end cloneComponents()


	/**
	* Perform any additional processing required during the cloning of this asset
	*
	* This function is being called by asset_manager after the dependants of asset
	* have been cloned.
	*
	* @param Asset	$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_schemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data',
	*							);
	*							or alternately
	*							Array('all');
	*
	* @return boolean
	* @access public
	*/
	public function cloneComponentsAdditional(Asset $clone, Array $components)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$GLOBALS['SQ_CLONE_COMPONENTS'] = TRUE;

		$bodycopy = $clone->getFormatBodycopy(FALSE);
		if (!is_null($bodycopy)) {
			$originals_clones[]	= Array (
									'original'	=> $this->id,
									'clone'		=> $clone->id,
								  );
			$replace_keywords = Array ( 'question_name', 'response', 'question_field', 'question_id', 'question_note', 'question_label', 'section_title');
			$form = $this->getParentForm();

			if ($form) {
				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);
				$containers = $bodycopy->getContainers();
				foreach ($containers as $container) {
					$content_type = $container->getContentType();
					if ($container->type() == 'bodycopy_table') {
						if (is_array($content_type)) {
							foreach ($content_type as $cell_type) {
								if (!$form->_cloneDIVContents($cell_type, $originals_clones, $replace_keywords)) {
									$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
									$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
									$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
									return FALSE;
								}//end if
							}//end foreach
						} else {
							if (!$form->_cloneDIVContents($content_type, $originals_clones, $replace_keywords)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
								$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
								return FALSE;
							}//end if
						}//end if
					} else {
						if (!$form->_cloneDIVContents($content_type, $originals_clones, $replace_keywords)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							return FALSE;
						}//end if
					}//end if
				}//end foreach
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}//end if
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;

		return $clone->fileRegeneration();

	}//end cloneComponentsAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $shadowid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		// Questions follow the format '<assetid>:q<shadowid>'
		if ($shadowid{0} == 'q') {

			// pick up the question in question (!)
			$questions = $this->attr('questions');
			$questionid = substr($shadowid, 1, strlen($shadowid));
			if (empty($questions[$questionid])) return $asset;

			$q_type_code = $questions[$questionid]['question_type_code'];

			// not fussed about type code?
			if (empty($type_code)) $type_code = $q_type_code;

			// only give the asset back if of the right type code
			if (in_array($q_type_code, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE))) {
				$questions[$questionid]['questionid'] = $questionid;
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($q_type_code);

				// this eval returns the necessary object for the question type
				$asset = new $q_type_code($this->id, $questions[$questionid]);
			}
		}

		return $asset;

	}//end getAsset()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link were returning
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		if (!is_numeric($assetid)) {
			if ($side_of_link == 'minor') {
				return Array(
						'linkid'			=> 0,
						'majorid'			=> $this->id,
						'minorid'			=> $this->id.':'.$assetid,
						'major_type_code'	=> $this->type(),
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2,
						'is_dependant'		=> TRUE,
						'is_exclusive'		=> FALSE,
						'sort_order'		=> 0,
						'locked'			=> 0,
					   );
			}
			return Array();
		}

		$id_parts = explode(':', $linkid);
		if (isset($id_parts[1]) && isset($id_parts[2])) {
			$looking_for = $id_parts[1].':'.$id_parts[2];
			$tmp_question_nb = str_replace('q', '', $id_parts[2]);
			$questions = $this->attr('questions');
			if (isset($questions[$tmp_question_nb])) {
				return Array(
						'linkid'			=> $linkid,
						'majorid'			=> $this->id,
						'minorid'			=> $this->id.':q'.$tmp_question_nb,
						'minor_type_code'	=> $questions[$tmp_question_nb]['question_type_code'],
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2,
						'is_dependant'		=> TRUE,
						'is_exclusive'		=> FALSE,
						'sort_order'		=> 0,
						'locked'			=> 0,
					   );
			}
		}

		return Array();

	}//end getLinkById()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		$tmp_questions = $this->attr('questions');
		if (isset($tmp_questions)) {
			return count($tmp_questions);
		}
		return 0;

	}//end countLinks()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		$id_parts = explode(':', $linkid);
		if (isset($id_parts[1]) && isset($id_parts[2])) {
			$question_to_delete = $id_parts[1].':'.$id_parts[2];
			$asset_to_delete = $this->getAsset($question_to_delete);
			if ($asset_to_delete != NULL) {
				return $this->deleteQuestion($asset_to_delete);
			}
		}

		return FALSE;

	}//end deleteAssetLink()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		// there are no links away from the shadow asset (ie:questions)
		$links = Array();

		if (!is_numeric($assetid)) {
			if ($side_of_link == 'minor') {
				$return_link = FALSE;

				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$return_link = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$return_link = ($type_code == $this->type());
				} else {
					$return_link = (in_array($this->type(), $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
				}

				if ($return_link) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':'.$assetid,
								'major_type_code'	=> $this->type(),
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
								'locked'			=> 0,
							   );
				}
			}

			return $links;
		}

		$new_sort_order = 0;

		// There are no real links made at a section level
		// but we also need question shadow asset links
		if ($link_types & SQ_LINK_TYPE_2) {
			$questions = $this->attr('questions');

			// keep a cache of asset type codes if we are checking on a non-strict
			// type code, to stop us calling getTypeDescendants() all the time
			// (not needed if strict type check or if not fussed)
			$get_question_types = Array();

			foreach ($questions as $questionid => $data) {
				if ($type_code == '') {
					// not fussed what we're getting, so return me
					$get_question = TRUE;
				} else if ($strict_type_code) {
					// strict type code check
					$get_question = ($type_code == $data['question_type_code']);
				} else {
					// if we've already cached whether this question type is to be
					// returned, then we don't need to look it up again
					if (isset($get_question_types[$data['question_type_code']])) {
						$get_question = $get_question_types[$data['question_type_code']];
						$get_question_types[$data['question_type_code']] = $get_question;
					} else {
						$get_question = (in_array($data['question_type_code'], $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE)));
					}
				}

				if ($get_question) {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':q'.$questionid,
								'minor_type_code'	=> $data['question_type_code'],
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> $new_sort_order,
								'locked'			=> 0,
							   );
				}
				$new_sort_order++;
			}//end foreach
		}//end if

		if (is_null($sort_by)) $sort_by = 'sort_order';

		uasort($links, function($a, $b) use ($sort_by) {
			return $a[$sort_by] > $b[$sort_by];
		});

		return $links;

	}//end getLinks()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$new_sort_order = 0;
		$old_links = $this->getLinks($this->id, SQ_SC_LINK_BACKEND_NAV);
		$links = Array();
		while (!empty($old_links)) {
			array_push($links, array_shift($old_links));
		}

		$questions = $this->attr('questions');

		foreach (array_keys($links) as $i) {

			$link =& $links[$i];

			// we would remove real links at this point, but Form Section doesn't
			// have any real links underneath it

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = 0;
			$link['accessible'] = 1;

			$link['assetid'] = $link['minorid'];
			$link['type_code'] = $link['minor_type_code'];
			$link['linkid'] = $link['majorid'].':'.$link['minorid'];

			$questionid = str_replace($this->id.':q', '', $link['assetid']);

			// make name and short name the same
			$link['name'] = $questions[$questionid]['attributes']['name'];
			$link['short_name'] = $link['name'];

			// make the status the same as the form's one
			$link['status'] = $this->status;

			unset($link['minor_type_code']);
			unset($link['majorid']);
			unset($link['minorid']);
			unset($link['value']);
			unset($link['is_exclusive']);

		}//end foreach

		return $links;

	}//end getAssetMapLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$shadowid			the shadow ID of the shadow asset to get (ie. the
	*											part after the ':')
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($shadowid, $type_code='', $strict_type_code=TRUE)
	{
		// basically get the parents of the section, and add itself
		$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetParentsQuery($this->id, $type_code, $strict_type_code);
		if (empty($ret_val)) return Array();

		$query = MatrixDAL::preparePdoQuery(implode(' ', $ret_val['sql_array']));
		foreach ($ret_val['bind_vars'] as $bind_var => $bind_value) {
			MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
		}

		$queried_parents = MatrixDAL::executePdoAssoc($query);
		$parents = Array();

		foreach ($queried_parents as $queried_parent) {
			$parents[$queried_parent['majorid']] = $queried_parent['type_code'];
		}

		// TODO: add this asset only if it meets type code requirements
		$parents[$this->id] = $this->type();

		return $parents;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* returns array(int => string)	returns an array of assetids and their type code
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		// no shadow assets have children in this asset
		if (!is_numeric($assetid)) return Array();

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		$children = Array();

		// we don't have to get any real links, sections only have question children

		// now questions
		$entries = $this->attr('questions');
		if (empty($type_code)) {
			foreach ($entries as $questionid => $data) {
				$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['question_type_code']));
			}
		} else {
			foreach ($type_code as $this_type_code) {
				foreach ($entries as $questionid => $data) {
					if ($strict_type_code) {
						if ($data['question_type_code'] == $this_type_code) {
							$children[$this->id.':q'.$questionid] = $data['question_type_code'];
						}
					} else {
						$type_desc = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($this_type_code) + Array($this_type_code);
						if (in_array($data['question_type_code'], $type_desc)) {
							$children[$this->id.':q'.$questionid] = Array(Array('type_code' => $data['question_type_code']));
						}
					}
				}
			}
		}

		if (!is_null($sort_by)) {
			uasort($children, create_function('$a,$b','return $a["'.$sort_by.'"] > $b["'.$sort_by.'"]'));
		}

		return $children;

	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the urls to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


//--        CONTENT FILE GENERATION        --//


	/**
	* regenerates the content file for this section
	*
	* @param boolean	$form_regen	if TRUE the section will get all its forms to regen as well
	*
	* @return boolean
	* @access public
	*/
	function fileRegeneration($form_regen=TRUE)
	{
		// we want to regen this section
		$edit = $this->getEditFns();
		$edit->generateContentFile($this);

		// sometimes, we might not want to regen the form (eg. if the form is calling this function!)
		if ($form_regen) {
			// then, get all the links of this guy's form parents
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form', FALSE, 'minor');

			foreach ($links as $l) {
				$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($l['majorid'], $l['major_type_code']);
				if (is_null($form)) continue;
				if (!$form->_updated()) {
					unset($form);
					return FALSE;
				}
				unset($form);
			}
		}//end if

		return TRUE;

	}//end fileRegeneration()




	/**
	* writes JavaScript code for client side validation
	*
	* This function assumes the skeleton of a JS validation function has been created around this code
	* with a form variable named 'form' passed (this is done when generating the content file).
	*
	* @return string
	* @access public
	*/
	function generateJSCode()
	{
		$questions = $this->getQuestions();
		$code = '';						// blank sheet of code to start with

		foreach ($questions as $q_id => $question) {
			$q_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_code = $q_asset->generateJSCode();

			$code .= $q_code.(empty($q_code) ? '' : "\n");
		}

		return $code;

	}//end generateJSCode()


//--        QUESTION MANAGEMENT        --//


	/**
	* get the section links attached to this form
	*
	* @param boolean	$force	whether to force... something or not
	*
	* @return array
	* @access public
	*/
	function getQuestions($force=FALSE)
	{
		return $this->attr('questions');

	}//end getQuestions()


	/**
	* Get the question assets directly attached to this section
	*
	* @param string	$type_code	When specified, only return assets of this exact type
	*
	* @return array
	* @access public
	*/
	function getQuestionAssets($type_code=NULL)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$result = Array();

		$questions = $this->attr('questions');

		if (!empty($questions)) {
			foreach ($questions as $q_id => $question) {
				if (empty($type_code) || ($question['question_type_code'] === $type_code)) {
					$id           =  $this->id.':q'.$q_id;
					$result[$id]  = $am->getAsset($id);
				}
			}
		}

		return $result;

	}//end getQuestionAssets()


	/**
	* Get all question assets attached to this section, including nested sections
	*
	* @param string	$type_code	When specified, only return assets of this exact type
	*
	* @return array
	* @access public
	*/
	function getAllQuestionAssets($type_code=NULL)
	{
		$questions = $this->getQuestionAssets($type_code);

		// Get questions within nested sections
		$nested_sections = $this->getSectionLinks();
		foreach ($nested_sections as $section_link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);
			$questions += $section->getAllQuestionAssets($type_code);
		}

		return $questions;

	}//end getQuestionAssets()

	/**
	* get a specific question
	*
	* @param int	$questionid	The id of the desired question
	*
	* @return array
	* @access public
	*/
	function getQuestionByID($questionid)
	{
		$questions = $this->getQuestions();
		return (isset($questions[$questionid])) ? $questions[$questionid] : Array();

	}//end getQuestionByID()


	/**
	* get specific question by order
	*
	* @param int	$orderid	the sequence of the desired question
	*
	* @return array
	* @access public
	*/
	function &getQuestionByOrder($orderid)
	{
		$sort_order = $this->attr('sort_order');
		$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($sort_order[$orderid]);
		return $question;

	}//end getQuestionByOrder()


	/**
	* attach a question to this form
	*
	* FALSE if database problem or question already attached, TRUE if sucessful
	*
	* @param string	$type_code	the type of the question to attach
	*
	* @return boolean
	* @access public
	*/
	function attachQuestion($type_code)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$questions = $this->getQuestions();
		if ($this->attr('next_questionid') > 0) {
			$new_key = $this->attr('next_questionid');
		} else if (empty($questions)) {
			$new_key = 1;
		} else {
			$new_key = max(array_keys($questions)) + 1;
		}

		$questions[$new_key] = Array(
								'question_type_code'	=> $type_code,
								'attributes'			=> Array(
															'name'	=> 'Question '.(count($questions)+1),
														   ),
							   );

		$this->setAttrValue('questions', $questions);
		$this->setAttrValue('next_questionid', $new_key + 1);

		$sort_order = $this->attr('sort_order');

		if (empty($sort_order)) {
			$new_index = 0;
		} else {
			$new_index = max(array_keys($sort_order))+1;
		}
		$sort_order[$new_index] = $this->id.':q'.$new_key;
		$this->setAttrValue('sort_order', $sort_order);

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end attachQuestion()


	/**
	* delete a question to this form
	*
	* returns FALSE if database problem or question already deleted, TRUE if sucessful
	*
	* @param object	&$question	the question object to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteQuestion(&$question)
	{
		if ($question->_questionid == 0) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$questions = $this->getQuestions();

		unset($questions[$question->_questionid]);

		$this->setAttrValue('questions', $questions);

		$sort_order = $this->attr('sort_order');

		// remove it from the sort order - it does not matter that there are gaps
		$old_index = array_search($question->id, $sort_order);
		unset($sort_order[$old_index]);

		// unset the question
		unset($question);

		$this->setAttrValue('sort_order', $sort_order);

		if (!$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteQuestion()


	/**
	* returns the count of questions linked to this section
	*
	* @return int
	* @access public
	*/
	function getQuestionCount()
	{
		$questions = $this->getQuestions();
		return count($questions);

	}//end getQuestionCount()


//--        SECTION MANAGEMENT        --//


	/**
	* get the links to any form sections that are nested within this form section
	*
	* @return array
	* @access public
	*/
	function getSectionLinks()
	{
		// This is what one would find in Asset_Manager::&getAsset() except we can't call that,
		// lest we spin off into an infinite loop involving OUR &getAsset().
		$value = Array(
					'link_value' 	=> Array(''),
					'equal' 		=> TRUE,
				 );
		$sql_query = $GLOBALS['SQ_SYSTEM']->am->generateGetLinksQuery($this->id, SQ_LINK_TYPE_2, 'form_section', TRUE, 'major', $value, '1', '0');
		if (empty($sql_query)) return Array();
		$sql_query['sql_array']['select'] .= ', l.majorid';		// we need the major id too
		$db = MatrixDAL::getDb();

		try {
			$sql = implode(' ', $sql_query['sql_array']);
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($sql_query['bind_vars'] as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get section links attached to form_section: '.$this->id.' due to database error: '.$e->getMessage());
		}
		return $result;

	}//end getSectionLinks()


	/**
	* returns immediate sections under this section in order of 'sort_order'
	*
	* Structure of return array is Array(object Form_Section).
	*
	* @return array
	* @access public
	*/
	function &getSections()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$section_links = $this->getSectionLinks();
		$sections = Array();

		foreach ($section_links as $link) {
			$sections[$link['sort_order']] = $am->getAsset($link['minorid'], $link['minor_type_code']);
		}
		return $sections;

	}//end getSections()


//--        PAINTING FUNCTIONS        --//


	/**
	* Paint the section contents.
	*
	* @param Form		$form				The form we are printing from. We need to know this,
	*										since the section can be shared amongst other forms,
	*										but some settings are made at a Form level, such as
	*										client side validation.
	*
	* @return void
	* @access public
	*/
	function printBody(Form $form)
	{
		// if format bodycopy enabled, print with that, otherwise print default
		if ($this->isFormatBodycopyEnabled()) {
			$this->printCustomFormat($form);
		} else {
			$this->printDefaultFormat($form);
		}

	}//end printBody()


	/**
	* Paints the form section using a default bodycopy
	*
	* @param Form		$form				The form we are printing from
	*
	* @return void
	* @access private
	*/
	function printDefaultFormat(Form $form)
	{
		include $this->data_path.'/content_file.php';

	}//end printCustomFormat()


	/**
	* Paints the form section using a custom format bodycopy
	*
	* @param Form		$form				The form we are printing from
	*
	* @return void
	* @access private
	*/
	function printCustomFormat(Form $form)
	{
		// return custom bodycopy but ONLY if enabled
		$format_bodycopy = $this->getFormatBodycopy(TRUE);
		if (!$format_bodycopy) {
			// Halt the display if the custom format is being asked for, but does
			// not exist
			trigger_error('Form Section asked for a custom format when it has no format bodycopy enabled. This should not happen.', E_USER_ERROR);
		}

		$keywords = $format_bodycopy->getKeywords();
		$replacements = Array();

		foreach ($keywords as $keyword) {
			// Try to get a specific display keyword replacement, to handle
			// %question_*% and nested %section_*% keywords
			$replacement = $form->getDisplayKeywordReplacement($keyword);

			if (is_null($replacement)) {
				// If we couldn't get a keyword from there, use the generic
				// keyword replacement form - this will pick up CAPTCHA, errors,
				// submit/reset replacements
				$replacement = $form->getKeywordReplacement($keyword);
			}

			if (!is_null($replacement)) {
				$replacements[$keyword] = $replacement;
			}

		}//end foreach

		$format_bodycopy->setKeywordReplacements($replacements);
		$format_bodycopy->printBody();

	}//end printCustomFormat()


	function printSemanticSummary()
	{
		if ($this->getQuestionCount() > 0) {
			echo '<dl>'."\n";
			foreach ($this->attr('sort_order') as $sort_order => $question_assetid) {

				// Make sure that we own this question
				if ($this->id == strtok($question_assetid, ':')) {
					$question = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_assetid);
					echo '<dt>'.$question->attr('name').'</dt>'."\n";
					echo '<dd>'.htmlentities($question->getSummary(), ENT_NOQUOTES).'</dd>'."\n";
				}

			}
			echo '</dl>'."\n";
		}

		// now nested sections - get them to print itself out, recursively
		$section_links = $this->getSectionLinks();
		if (count($section_links) > 0) {
			echo '<ul>'."\n";
		}

		foreach ($section_links as $section_link) {
			echo '<li>'."\n";
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);
				echo '<strong>'.htmlentities($section->attr('name'), ENT_NOQUOTES).'</strong>'."\n";
				$section->printSemanticSummary();
			echo '</li>'."\n";
		}

		if (count($section_links) > 0) {
			echo '</ul>'."\n";
		}

	}//end printSemanticSummary()


//--        CUSTOM FORMAT BODYCOPY FUNCTIONS        --//


	/**
	* Creates the format bodycopy
	*
	* Returns false if bodycopy already exists
	*
	* @param boolean	$enable_on_create	if true, will set the bodycopy to be enabled
	*										(TYPE_2 link) when it is created. If false,
	*										creates it disabled (TYPE_3 link).
	*
	* @return boolean
	* @access public
	*/
	function createFormatBodycopy($enable_on_create=TRUE)
	{
		$_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', 'format');
		$bodycopy_link = reset($_bodycopy_link);

		// we already have a bodycopy link?!
		if ($bodycopy_link) {
			return FALSE;
		} else {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

			$link_type = ($enable_on_create ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3);

			$asset = new Bodycopy();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> 'format',
							'link_type'		=> $link_type,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						 );

			$asset->setAttrValue('name', 'Format Bodycopy');
			if (!$asset->create($copy_link)) return FALSE;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		return TRUE;

	}//end createFormatBodycopy()


	/**
	* Returns the format bodycopy or NULL
	*
	* @param boolean	$only_if_enabled	if true, will return NULL if the bodycopy is
	*										disabled. If false, will only return NULL if
	*										never created.
	*
	* @return object
	* @access public
	*/
	function &getFormatBodycopy($only_if_enabled=TRUE)
	{
		$asset = NULL;

		if ($only_if_enabled) {
			$link_types = SQ_LINK_TYPE_2;
		} else {
			$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;
		}

		$tmp_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', 'format');
		$bodycopy_link = reset($tmp_bodycopy);

		if ($bodycopy_link) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], 'bodycopy');
		}

		return $asset;

	}//end getFormatBodycopy()


	/**
	* Returns whether the format bodycopy is enabled
	*
	* @return boolean
	* @access public
	*/
	function isFormatBodycopyEnabled()
	{
		$link_types = SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3;

		$_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, $link_types, 'bodycopy', 'format');
		$bodycopy_link = reset($_bodycopy_link);

		if ($bodycopy_link) {
			return ($bodycopy_link['link_type'] == SQ_LINK_TYPE_2);
		} else {
			return FALSE;
		}


	}//end isFormatBodycopyEnabled()




	/**
	* Returns all format bodycopies including current section and all nested sections
	*
	* @return object
	* @access public
	*/
	function getNestedFormatBodycopies()
	{
		$bodycopies = Array();

		$bodycopy = $this->getFormatBodycopy();
		if(!empty($bodycopy))
			$bodycopies[] = $bodycopy->id;

		$nested_sections = $this->getSectionLinks();
		foreach ($nested_sections as $section_link) {
			$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_link['minorid']);
			$bodycopies = array_merge($bodycopies, $section->getNestedFormatBodycopies());
		}

		return $bodycopies;

	}//end getNestedFormatBodycopies()



	/**
	* Handles the enabling or disabling of the format bodycopy
	*
	* @param boolean	$enabled	set to true to enable the format bodycopy,
	*								set to false to disable it
	*
	* @return boolean
	* @access public
	*/
	function setUseFormatBodycopy($enabled)
	{
		$tmp_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'bodycopy', 'format');
		$bodycopy_link = reset($tmp_bodycopy);

		if (!$bodycopy_link) {
			// no bodycopy yet? If we're trying to set to disabled, then we don't
			// need to do anything - if not then we need to create it
			if ($enabled) {
				if (!$this->createFormatBodycopy()) return FALSE;
			}
		} else {
			// set link type to either TYPE_2 if enabled or TYPE_3 if disabled
			$new_link_type = ($enabled) ? SQ_LINK_TYPE_2 : SQ_LINK_TYPE_3;
			if ($bodycopy_link['link_type'] != $new_link_type) {
				$GLOBALS['SQ_SYSTEM']->am->updateLink($bodycopy_link['linkid'], $new_link_type);
				$this->fileRegeneration();
			}
		}

		return TRUE;

	}//end setUseFormatBodycopy()


//--        KEYWORD PROVISION        --//


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$vars['keywords'] = isset($vars['keywords']) ? $vars['keywords'] : Array();
		$keywords = Array();

		if ($this->isFormatBodycopyEnabled()) {
			$keywords = $this->getDisplayKeywords();
		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);


		// get conditions keywords from page custom form
		$parent_asset = $this->getParentForm();
		$keywords = $parent_asset->getAvailableConditionKeywords();
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Returns a list of display keywords for the questions handled by this section
	*
	* return format is (string => string)	keyword => description
	*
	* @param boolean	$include_submit_keywords	If TRUE, keywords for submit/reset/errors will
	*												be made available. If calling from a single-page
	*												form, this will probably be FALSE since the form
	*												provides its own.
	*
	* @return array
	* @access public
	*/
	function getDisplayKeywords($include_submit_keywords=TRUE)
	{
		$questions = $this->attr('questions');
		$keywords = Array();

		foreach ($questions as $shadowid => $question) {
			$keywords['question_field_'.$this->id.'_q'.$shadowid] = 'Question Field: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_name_'.$this->id.'_q'.$shadowid]    = 'Question Name: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_id_'.$this->id.'_q'.$shadowid]    = 'Question ID: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_note_'.$this->id.'_q'.$shadowid]  = 'Question Note: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_label_'.$this->id.'_q'.$shadowid] = 'Question Label: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_error_'.$this->id.'_q'.$shadowid] = 'Question Error: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_contents_'.$this->id.'_q'.$shadowid] = 'Question Content: '.ellipsisize($question['attributes']['name'], 30).' ('.$this->id.':q'.$shadowid.')';
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form_section', FALSE) as $link) {
			// get the section
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);

			$keywords += $s->getDisplayKeywords();
			$keywords['section_title_'.$s->id] = 'Nested Section Name: '.$s->name.' ('.$s->id.')';
			$keywords['section_contents_'.$s->id] = 'Nested Section Contents: '.$s->name.' ('.$s->id.')';

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		if ($include_submit_keywords) {
			$keywords['submit_button'] = 'Submit Button';
			$keywords['reset_button'] = 'Reset Button';
			$keywords['form_errors'] = 'Form Errors';
		}

		return $keywords;

	}//end getDisplayKeywords()


	/**
	* Returns a list of response keywords for the questions handled by this section
	*
	* Return format is (string => string)	keyword => description
	*
	* @return array
	* @access public
	*/
	function getResponseKeywords()
	{
		$questions = $this->attr('questions');
		$keywords = Array();

		foreach ($questions as $shadowid => $question) {
			$keywords['response_'.$this->id.'_q'.$shadowid]       = translate('cms_form_section_response', $this->name, ellipsisize($question['attributes']['name'], 30)).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_name_'.$this->id.'_q'.$shadowid]  = translate('cms_form_section_q_name', $this->name, ellipsisize($question['attributes']['name'], 30)).' ('.$this->id.':q'.$shadowid.')';
			$keywords['question_note_'.$this->id.'_q'.$shadowid]  = translate('cms_form_section_q_note', $this->name, ellipsisize($question['attributes']['name'], 30)).' ('.$this->id.':q'.$shadowid.')';
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form_section', FALSE) as $link) {
			// get the nested form
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);

			$keywords += $s->getResponseKeywords();
			$keywords['section_title_'.$s->id] = 'Nested Section Name: '.$s->name.' ('.$s->id.')';

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		return $keywords;

	}//end getResponseKeywords()


//--        KEYWORD REPLACEMENTS        --//


	/**
	* Returns a list of display keywords for the questions handled by this section
	*
	* Return format is (string => string)	keyword => description
	*
	* @return array
	* @access public
	*/
	function getDisplayKeywordReplacements($generating=FALSE, $include_submit_keywords=TRUE, $form=NULL)
	{
		if (!is_null($form)) {
			assert_is_a($form, 'Form');
		}

		$questions = $this->attr('questions');
		$keywords = Array();

		foreach ($questions as $shadowid => $question) {
			// get the question
			$assetid = $this->id.':q'.$shadowid;
			$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if ($generating) {
				$keywords['question_field_'.$this->id.'_q'.$shadowid] = '<'.'?php echo $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$assetid.'\')->getHtmlField(); ?'.'>';
				$keywords['question_note_'.$this->id.'_q'.$shadowid]  = '<'.'?php echo $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$assetid.'\')->attr(\'note\'); ?'.'>';
				$keywords['question_name_'.$this->id.'_q'.$shadowid]  = '<'.'?php echo $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$assetid.'\')->attr(\'name\'); ?'.'>';
				$keywords['question_id_'.$this->id.'_q'.$shadowid]    = $assetid;
				$keywords['question_label_'.$this->id.'_q'.$shadowid] = '<'.'?php echo $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$assetid.'\')->getHtmlLabel(); ?'.'>';
				$keywords['question_error_'.$this->id.'_q'.$shadowid]    = '<'.'?php 
					$question = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$assetid.'\');
					if(isset($form_asset->question_errors[$question->id])) {
						$errors = $form_asset->question_errors[$question->id];
						$error = array_pop($errors);
						echo $error;
					}
				 ?'.'>';
				$keywords['question_contents_'.$this->id.'_q'.$shadowid] = $this->getQuestionContentReplacement(TRUE, $q->id);
			} else {
				$keywords['question_field_'.$this->id.'_q'.$shadowid] = $q->getHtmlField();
				$keywords['question_note_'.$this->id.'_q'.$shadowid]  = $q->attr('note');
				$keywords['question_name_'.$this->id.'_q'.$shadowid]  = $q->attr('name');
				$keywords['question_id_'.$this->id.'_q'.$shadowid]    = $assetid;
				$keywords['question_label_'.$this->id.'_q'.$shadowid] = $q->getHtmlLabel();
				$parent_form = $this->getParentForm();
				if(isset($parent_form->question_errors[$q->id])) {
					$errors = $parent_form->question_errors[$q->id];
					$error = array_pop($errors);
					$keywords['question_error_'.$this->id.'_q'.$shadowid]    = $error;
				}
				$keywords['question_contents_'.$this->id.'_q'.$shadowid] = $this->getQuestionContentReplacement(FALSE, $q->id);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
			unset($q);
		}

		foreach ($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'form_section', FALSE) as $link) {
			// get the nested form section
			$sectionid = $link['minorid'];
			$s = $GLOBALS['SQ_SYSTEM']->am->getAsset($sectionid);
			$keywords += $s->getDisplayKeywordReplacements($generating, FALSE);

			if ($generating) {
				$keywords['section_title_'.$s->id] = '<'.'?php echo $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$sectionid.'\')->name; ?'.'>';;

				$keywords['section_contents_'.$s->id] = '<'.'?php include_once $GLOBALS[\'SQ_SYSTEM\']->am->getAsset(\''.$sectionid.'\')->data_path.\'/content_file.php\'; ?'.'>';;
			} else {
				$keywords['section_title_'.$s->id] = $s->name;
				ob_start();
					$s->printBody($form);
				$keywords['section_contents_'.$s->id] = ob_get_clean();
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($s);
			unset($s);
		}

		if ($include_submit_keywords) {
			// Submit Button, Reset Button, Form Errors
			// TODO:
			ob_start();
				//submit_button('submit_button', );
				$submit_button = ob_get_contents();
			ob_end_clean();

			ob_start();
				//reset_button('reset_button', 'Reset');
				$reset_button = ob_get_contents();
			ob_end_clean();

			$keywords['submit_button'] = $submit_button;
			$keywords['reset_button']  = $reset_button;

			// TODO:
			$keywords['form_errors']   = implode('\n', $this->submission_errors);
		} else {
			// suppress them
			$keywords['submit_button'] = '';
			$keywords['reset_button']  = '';
			$keywords['form_errors']   = '';
		}

		return $keywords;

	}//end getDisplayKeywordReplacements()


	/**
	* Check if parent form's using accessible format
	*
	*
	* @return boolean
	* @access public
	*/
	function useAccessibleFormat()
	{
		$parent = $this->getParentForm();
		if(is_null($parent)) return TRUE;
		return $parent->useAccessibleFormat();
	}//end useAccessibleFormat()


	/**
	* Get required field mark attribute
	*
	*
	* @return boolean
	* @access public
	*/
	function getRequiredFieldMark()
	{
		if(!$this->useAccessibleFormat()) return '';
		$parent = $this->getParentForm();
		if(is_null($parent)) return  ' '.'<abbr class="sq-form-required-field" title="required">*</abbr>';
		return $parent->getRequiredFieldMark();
	}//end getRequiredFieldMark()


	/**
	* Get parent form / section
	*
	*
	* @return object
	* @access public
	*/
	function getParentForm()
	{
	    if(!empty($this->parent_form_id)) {
			$parent_form_id = $this->parent_form_id;
	    } else {
			$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form', FALSE, NULL, 'minor');
			if(!isset($form_link['majorid'])) {
			    $form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'form_section', FALSE, NULL, 'minor');
			}
			// if this form section is not linked to any asset
			if(!isset($form_link['majorid'])) {
			    return NULL;
			}
			$parent_form_id = $form_link['majorid'];
			$this->parent_form_id = $parent_form_id;
	    }

	    return $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_form_id);

	}//end getParentForm


	/**
	* Get the output/keyword replacement for question content including label input and everything
	* @param boolean	$generating	if it's for generating a file or dynamic replacement
	* @param int        $assetid    assetid of the question
	*
	* @return string
	* @access public
	*/
	function getQuestionContentReplacement($generating=FALSE, $assetid)
	{
		$output = '';
		$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		// skip over if asset doesn't exist anymore
		if (is_null($q)) return '';

		$type = str_replace('_', '-', str_replace('form_question_type_', '', $q->type()));
		// use fieldset to group complex questions
		$isFieldset = $q->useFieldset();

		if($generating) {
			$output .= '<'.'?php $question = $GLOBALS["SQ_SYSTEM"]->am->getAsset("'.$assetid.'"); ?'.'>'."\n";
			
			// add error class
			$output .= '<'.'?php 
			$error_class = isset($form_asset->question_errors[$question->id]) ? "sq-form-question-error" : "";
			?'.'>'."\n";
			// question wrapper
			$output .= '<div class="sq-form-question sq-form-question-'.$type.' <?php echo $error_class; ?>">';
			if($isFieldset)
				$output .= '<fieldset>';
			// label
			$label   = $q->getHtmlLabel();
			$label .= ' ';
			escape_php($label);
			$output .= $label;
				    // note
			if ($q->getVal('note')) {
				$output .= '<em class="sq-form-question-note">'.$q->getVal('note').'</em> ';
			}
			// question content
			$output .= '<div class="sq-form-question-answer" ><'.'?php echo $question->getHtmlField(); ?'.'></div>'."\n";
			// question error
			$output .= '<'.'?php 
			if(isset($form_asset->question_errors[$question->id])) {
				$errors = $form_asset->question_errors[$question->id];
				$error = array_pop($errors);
				echo " <p id=\"error_".str_replace(":", "_", $question->id)."\" class=\"sq-form-error\"  >".$error."</p>";
			}
			?'.'>'."\n";
			if($isFieldset)
				$output .= '</fieldset>';
			$output .= '</div>';
		}
		else {
			$parent_form = $this->getParentForm();
			$is_error = isset($parent_form->question_errors[$q->id]);
			$error_class = $is_error ? 'sq-form-question-error' : '';

			$output .= '<div class="sq-form-question sq-form-question-'.$type.' '.$error_class.'">';
			if($isFieldset)	$output .= '<fieldset>';
			$label   = $q->getHtmlLabel();
			$output .= $label.' ';
			// note
			if ($q->attr('note')) {
				$output .= '<em class="sq-form-question-note">'.$q->attr('note').'</em> ';
			}
			// question content
			$output .= '<div class="sq-form-question-answer" >'.$q->getHtmlField().'</div>'."\n";
			if($is_error) {
				$errors = $parent_form->question_errors[$q->id];
				$error = array_pop($errors);
				$output .= ' <p id="error_'.str_replace(":", "_", $q->id).'\" class="sq-form-error"  >'.$error.'</p>';
			}
			if($isFieldset) $output .= '</fieldset>';
			$output .= '</div>';
		}
		return $output;
	}//end getQuestionContentReplacement()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* If a single asset ID is passed the function will return TRUE or FALSE depending on
	* whether that asset exists. If an array is passed it will return an array of the
	* assetids that exist.
	*
	*
	* @param mixed	$assetids	the ID of the asset we are going to the check for existence
	*							or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		$sort_order = $this->attr('sort_order');

		if (is_array($assetids)) {
			$ret_val = Array();
	       	foreach ($assetids as $assetid) {
				$question_order = array_search($assetid, $sort_order);
				if ($question_order !== FALSE) {
					$question_asset = $this->getQuestionByOrder($question_order);
					if (!is_null($question_asset)) {
						$ret_val[] = $question_asset->id;
					}
				}
			}
		} else {
			$ret_val = FALSE;
			$question_order = array_search($assetids, $sort_order);
			$question_asset = $this->getQuestionByOrder($question_order);
			if (!is_null($question_asset)){
				$ret_val = TRUE;
			}
		}

		return $ret_val;

	}//end assetExists()




	/**
	*
	* Get available keywords based on condition set on the asset
	*
	*
	* @return array
	* @access public
	*/
	function getAvailableConditionKeywords()
	{
		$form = $this->getParentForm();
		return $form->getAvailableConditionKeywords();
	}



}//end class

?>
