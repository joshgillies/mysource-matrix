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
* $Id: thesaurus.inc,v 1.2 2004/10/28 03:39:28 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Thesaurus
*
* Purpose
*
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Thesaurus extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Thesaurus($assetid=0)
	{
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()



	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{

	}//end printBody()



	/**
	* Get all abbreviation relationship instances in this thesaurus. 
	*
	* THIS FUNCTION ONLY TO BE CALLED - AND ONLY TEMPORARILY - FROM THE MATRIX_EXPAND_KEYWORDS 
	* WYSIWYG PLUGIN.  OR ELSE!
	*
	* @return array (abbrev => expansion)
	* @access public
	*/
	function getAbbrevsDeprecated()
	{
		return Array(
			'AAA' =>	'abdominal aortic aneurysm',  	   	
			'AAS' =>	'acute abdominal series', 
			'ABD' =>	'abdomen',
			'ABG' =>	'arterial blood gas',
			'AC' =>		'before eating',
			'ACLS' =>	'advanced cardiac life support',
			'ACTH' =>	'adrenocorticotropic hormone',
			'ADH' =>	'anti-diuretic hormone', 	  	
			'ad lib' => 'as much as needed',
			'AF' =>		'atrial fibrillation',  	
			'AFB' =>	'acid-fast bacilli',
			'AFP' =>	'alpha-fetoprotein', 	  	
			'A/G' =>	'albumin/globulin ratio',
			'AI' =>		'aortic insufficiency', 	 	
			'AKA ' =>	'above the knee amputation',
			'ALL' =>	'acute lymphocytic leukemia', 	  	
			'amb' =>	'ambulate',
			'AML' =>	'acute myelogenous leukemia', 	  	
			'ANA' =>	'antinuclear antibody',
			'AOB' =>	'alcohol on breath', 	  	
			'AODM' =>	'adult onset diabetes mellitus',
			'AP' =>		'anteroposterior',  	
			'ARDS' => 	'acute respiratory distress syndrome',
			'ARF' =>	'acute renal failure', 	  	
			'AS' =>		'aortic stenosis',
			'ASAP' =>	'as soon as possible', 	  	
			'ASCVD' =>	'atherosclerotic cardiovascular disease',
			'ASD' =>	'atrial septal defect', 	  	
			'ASHD' =>	'atherosclerotic heart disease',
			'AV' =>		'atrioventricular', 	  	
			'A-V' =>	'arteriovenous',
			'A-VO2' =>	'arteriovenous oxygen', 	  	
			'BBB' =>	'bundle branch block',
			'BCAA' =>	'branched chain amino acids', 	  	
			'BE' =>		'barium enema',
			'BEE' =>	'basal energy expenditure', 	  	
			'bid' =>	'twice a day',
			'BKA' =>	'below the knee amputation', 	
			'BM' =>		'bowel movement',
			'BMR' =>	'basal metabolic rate', 	  	
			'BOM' =>	'bilateral otitis media',
			'BP' =>		'blood pressure', 	  	
			'BPH' =>	'benign prostatic hypertrophy',
			'BPM' =>	'beats per minute', 	  	
			'BRBPR' =>	'bright red blood per rectum',
			'BRP' =>	'bathroom priviledges', 	  	
			'BS' =>		'bowel',
			'BUN' =>	'blood urea nitrogen', 	  	
			'BW' =>		'body weight',
			'BX' =>		'with',
			'C&S' =>	'culture and sensitivity', 	  	
			'CA' =>		'cancer',
			'Ca' =>		'calcium', 	  	
			'CAA' =>	'crystalline amino acids',
			'CABG' =>	'coronary artery bypass graft', 	  	
			'CAD' =>	'coronary artery disease',
			'CAT' =>	'computerized axial tomography', 	
			'CBC' =>	'complete blood count',
			'CBG' =>	'capillary blood gas', 	  	
			'CC' =>		'chief complaint',
			'CCU' =>	'clean catch urine',
			'CCV' =>	'critical closing volume',
			'CF' =>		'cystic fibrosis', 	  	
			'CGL' =>	'chronic granulocytic leukemia',
			'CHF' =>	'congestive heart failure', 	  	
			'CHO' =>	'carbohydrate',
			'CI' =>		'cardiac index', 	  	
			'CML' =>	'chronic myelogenous leukemia',
			'CMV' =>	'cytomegalovirus', 	  	
			'CN' =>		'cranial nerves',
			'CNS ' =>	'central nervous system', 	  	
			'CO' =>		'cardiac output',
			'C/O' =>	'complaining of', 	 	
			'COLD' =>	'chronic obstructive lung disease',
			'COPD' =>	'chronic obstructive pulmonary disease', 	  	
			'CP' =>		'chest pain',
			'CPAP' =>	'continuous positive airway pressure', 	  	
			'CPK' =>	'creatinine phosphokinase',
			'CPR' =>	'cardiopulmonary resuscitation', 	  	
			'CRCL' =>	'creatinine clearance',
			'CRF' =>	'chronic renal failure', 	  	
			'CRP' =>	'C-reactive protein',
			'CSF' =>	'cerebrospinal fluid', 	  	
			'CT' =>		'computerized tomography',
			'CVA' =>	'cerebrovascular accident', 
			'CVAT' =>	'CVA tenderness',
			'CVP' =>	'central venous pressure', 	  	
			'CXR' =>	'chest X-ray',
			'DAT' =>	'diet as tolerated',  	
			'DAW' =>	'dispense as written',
			'DC' =>		'discontinue', 	  	
			'D&C' =>	'dilation and curettage',
			'DDx' =>	'differential diagnosis', 	  	
			'D5W' =>	'5% dextrose in water',
			'DI' =>		'diabetes insipidus', 	  	
			'DIC' =>	'disseminated intravascular coagulopathy',
			'DIP' =>	'distal interphalangeal joint', 	  	
			'DJD' =>	'degenerative joint disease',
			'DKA' =>	'diabetic ketoacidosis', 	  	
			'dL' =>		'deciliter',
			'DM' =>		'diabetes mellitus', 	  	
			'DNR ' =>	'do not resuscitate',
			'DOA' =>	'dead on arrival', 	  	
			'DOE' =>	'dyspnea on exertion',
			'DPL' =>	'diagnostic peritoneal lavage', 	  	
			'DPT' =>	'diphtheria, pertussis, tetanus',
			'DTR' =>	'deep tendon reflexes', 	  	
			'DVT' =>	'deep venous thrombosis',
			'DX' =>		'diagnosis', 	  	
			'EAA' =>	'essential amino acids',
			'EBL' =>	'estimated blood loss', 	 	
			'ECG' =>	'electrocardiogram',
			'ECT' =>	'electroconvulsive therapy', 	  	
			'EFAD' =>	'essential fatty acid deficiency',
			'EMG' =>	'Electromyogram', 	  	
			'EMV' =>	'eyes, motor, verbal response (Glasgow coma scale)',
			'ENT' =>	'ears, nose, and throat', 	  	
			'EOM' =>	'extraocular muscles',
			'ESR' =>	'erythrocyte sedimentation rate', 	  	
			'ET' =>		'endotracheal',
			'ETT' =>	'endotracheal tube', 	  	
			'ERCP' =>	'endoscopic retrograde cholangio -pancreatography',
			'ETOH' =>	'ethanol', 	  	
			'EUA' =>	'examination under anesthesia',
			'FBS' =>	'fasting blood sugar',
			'FEV' =>	'forced expiratory volume',
			'FFP' =>	'fresh frozen plasma', 	  	
			'FRC' =>	'functional residual capacity',
			'FTT' =>	'failure to thrive', 	  	
			'FU' =>		'follow-up',
			'FUO' =>	'fever of unknown origin', 	  	
			'FVC' =>	'forced vital capacity',
			'Fx' =>		'fracture', 	 	
			'GC' =>		'gonorrhea',
			'GETT' =>	'general by endotracheal tube', 	  	
			'GFR' =>	'glomerular filtration rate',
			'GI' =>		'gastrointestinal', 	  	
			'gr' =>		'grain',
			'GSW' =>	'gun shot wound', 	  	
			'gt ' =>	'drops',
			'GTT' =>	'glucose tolerance test', 	  	
			'GU' =>		'genitourinary',
			'GXT' =>	'graded exercise tolerance (Stress test)', 	  	
			'HA' =>		'headache',
			'HAA' =>	'hepatitis  B surface antigen', 	 	
			'HAV' =>	'hepatitis A virus',
			'HBP' =>	'high blood pressure', 	  	
			'HCG' =>	'human chorionic gonadotropin',
			'HCT' =>	'hematocrit', 	  	
			'HDL' =>	'high density lipoprotein',
			'HEENT' =>	'head, eyes, ears, nose, throat', 	  	
			'Hgb' =>	'hemoglobin',
			'H/H' =>	'henderson-hasselbach equation',	  	
			'HIV' =>	'human immunodeficiency virus',
			'HLA' =>	'histocompatibility locus antigen', 	  	
			'HJR' =>	'hepatojugular reflex',
			'HO' =>		'history of', 	  	
			'HOB' =>	'head of bed',
			'HPF' =>	'high power field', 	  	
			'HPI' =>	'history of present illness',
			'HR' =>		'heart rate', 	  	
			'HS' =>		'at bedtime',
			'HSM' =>	'hepatosplenomegaly', 	  	
			'HTLV-III' =>	'human lymphotropic virus, type III', 
			'HSV' =>	'herpes simplex virus',	  	
			'HTN' =>	'hypertension',
			'Hx' =>		'history',
			'I&D' =>	'incision and drainage',
			'I&O' =>	'intake and output', 	  	
			'ICS' =>	'intercostal space',
			'ICU' =>	'intensive care unit', 	  	
			'ID' =>		'infectious disease', 
			'IDDM' =>	'insulin dependent diabetes mellitus', 	  	
			'IG' =>		'immunoglobulin',
			'IHSS' =>	'idiopathic hypertropic subaortic stenosis', 	  	
			'IM' =>		'intramuscular',
			'IMV' =>	'intermittent mandatory ventilation', 	 	
			'INF' =>	'intravenous nutritional fluid',
			'IPPB' =>	'intermittent positive pressure breathing',
			'IRBBB' =>	'incomplete right bundle branch block',
			'IRDM' =>	'insulin resistant diabetes mellitus',
			'ITP' =>	'idiopathic thrombocytopenic purpura', 	  	
			'IV' =>		'intravenous',
			'IVC' =>	'intravenous cholangiogram',
			'IVP' =>	'intravenous pyelogram',			
			'tom' =>	'Tom Barrett',
		);

	}//end getAbbrevsDeprecated()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName()
	{
		return $this->attr('name');

	}//end _getName()

}//end class

?>