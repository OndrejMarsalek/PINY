/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/
/*                                                                          */
/*                         PI_MD:                                           */
/*             The future of simulation technology                          */
/*             ------------------------------------                         */
/*                      Module: simpavg_md                                  */
/*                                                                          */
/* This program calculates some simple averages of quantities               */
/* obtained from an MD on a classical potential energy surface (PES),       */
/*                                                                          */
/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/


#include "standard_include.h"
#include "../typ_defs/typedefs_class.h"
#include "../typ_defs/typedefs_bnd.h"
#include "../typ_defs/typedefs_gen.h"
#include "../typ_defs/typedefs_cp.h"
#include "../proto_defs/proto_output_cp_entry.h"
#include "../proto_defs/proto_output_entry.h"
#include "../proto_defs/proto_output_cp_local.h"
#include "../proto_defs/proto_output_local.h"
#include "../proto_defs/proto_communicate_wrappers.h"

#define DEBUG_CP_OFF

/*==========================================================================*/
/*cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc*/
/*==========================================================================*/

void simpavg_cp(TIMEINFO *timeinfo,STAT_AVG *stat_avg,CELL *cell, 
              CONSTRNT *constrnt,ENSOPTS *ensopts, SIMOPTS *simopts, 
              PTENS *ptens,CP *cp,COMMUNICATE *communicate,
              VERLIST *verlist,ENERGY_CTRL *energy_ctrl)

/*==========================================================================*/
/*             Begin Routine                                                */
       {/*begin routine*/

/*==========================================================================*/
/*             Local variable declarations                                  */

  int i,iii;
  double a,b,c,tab,tbc,tac,etot;        
  int myid       = communicate->myid;
  int nproc      = communicate->np;
  int np_states  = communicate->np_states;
  int np_forc    = communicate->np_forc;
  MPI_Comm world = communicate->world;

  int simopts_cp;

  simopts_cp = simopts->cp;
/*========================================================================*/
/* I) Zero the averages on the first time step */

if((timeinfo->itime)<=1){stat_avg->count_diag_srot = 0.0;}

if(myid==0){
  if((timeinfo->itime)==1){
    stat_avg->count_diag_srot = 0.0;
    stat_avg->aiter_shake_cp= 0.0;
    stat_avg->aiter_ratl_cp = 0.0;
    stat_avg->akinet_cp     = 0.0;
    stat_avg->akinet_nhc_cp = 0.0;
    stat_avg->acp_ehart     = 0.0;
    stat_avg->acp_eext      = 0.0;
    stat_avg->acp_exc       = 0.0;
    stat_avg->acp_eke       = 0.0;
    stat_avg->acp_enl       = 0.0;
    stat_avg->econv         = 0.0;
    stat_avg->cp_kconv      = 0.0;
    stat_avg->akinet        = 0.0;
    stat_avg->akinet_v      = 0.0;
    stat_avg->akinet_nhc    = 0.0;
    stat_avg->avintert      = 0.0;
    stat_avg->avintrat      = 0.0;
    stat_avg->avol          = 0.0;
    stat_avg->acella        = 0.0;
    stat_avg->acellb        = 0.0;
    stat_avg->acellc        = 0.0;
    stat_avg->acellab       = 0.0;
    stat_avg->acellbc       = 0.0;
    stat_avg->acellac       = 0.0;
    stat_avg->aiter_shake   = 0.0;
    stat_avg->aiter_ratl    = 0.0;
    stat_avg->aiter_21      = 0.0;
    stat_avg->aiter_23      = 0.0;
    stat_avg->aiter_33      = 0.0;
    stat_avg->aiter_46      = 0.0;
    stat_avg->aiter_43      = 0.0;
    stat_avg->aiter_21r     = 0.0;
    stat_avg->aiter_23r     = 0.0;
    stat_avg->aiter_33r     = 0.0;
    stat_avg->aiter_46r     = 0.0;
    stat_avg->aiter_43r     = 0.0;
    for(i=1;i<=9;i++){
      stat_avg->apten[i]    = 0.0;
    }/*endfor:i*/
    stat_avg->aikinet_cp    = 0.0;
    stat_avg->aikinet_nhc_cp= 0.0;
    stat_avg->aicp_ehart    = 0.0;
    stat_avg->aicp_eext     = 0.0;
    stat_avg->aicp_exc      = 0.0;
    stat_avg->aicp_eke      = 0.0;
    stat_avg->aicp_enl      = 0.0;
    stat_avg->apress        = 0.0;
    stat_avg->aikinet       = 0.0;
    stat_avg->aikinet_v     = 0.0;
    stat_avg->aikinet_nhc   = 0.0;
    stat_avg->aivintert     = 0.0;
    stat_avg->aivintrat     = 0.0;
    stat_avg->aivol         = 0.0;
    stat_avg->aicella       = 0.0;
    stat_avg->aicellb       = 0.0;
    stat_avg->aicellc       = 0.0;
    stat_avg->aicellab      = 0.0;
    stat_avg->aicellbc      = 0.0;
    stat_avg->aicellac      = 0.0;
    for ( i = 1;i<=9;i++){
      stat_avg->aipten[i]   = 0.0;
    }/*endfor:i*/
    stat_avg->aipress       = 0.0;
  }/*endif: first time*/
}/*endif:my_id*/
if(nproc>1){Barrier(world);}

/*======================================================================*/
/* II) Communication                                                    */

 if(np_states>1){
  simpavg_cp_communicate(cp,stat_avg,communicate,ptens);
 }/*endif*/


 if(np_forc > 1){
    simpavg_md_communicate(stat_avg,ensopts,ptens,
        constrnt->iconstrnt,communicate,verlist,timeinfo->int_res_ter,
        energy_ctrl->iget_pv_real_inter,energy_ctrl->iget_pe_real_inter);
 }/*endif*/

if(myid==0){

/*======================================================================*/
/*======================================================================*/
/* II) Shake/Rattle stuff */

  if((constrnt->iconstrnt)==1&&simopts_cp==1){
    stat_avg->aiter_shake += (double) stat_avg->iter_shake;
    stat_avg->aiter_ratl  += (double) stat_avg->iter_ratl;
    stat_avg->aiter_21    += stat_avg->iter_21;
    stat_avg->aiter_23    += stat_avg->iter_23;
    stat_avg->aiter_33    += stat_avg->iter_33;
    stat_avg->aiter_46    += stat_avg->iter_46;
    stat_avg->aiter_43    += stat_avg->iter_43;
    stat_avg->aiter_21r   += stat_avg->iter_21r;
    stat_avg->aiter_23r   += stat_avg->iter_23r;
    stat_avg->aiter_33r   += stat_avg->iter_33r;
    stat_avg->aiter_46r   += stat_avg->iter_46r;
    stat_avg->aiter_43r   += stat_avg->iter_43r;
  }/*endif*/
  stat_avg->aiter_shake_cp  += (double)(stat_avg->iter_shake_cp);
  stat_avg->aiter_ratl_cp += (double)(stat_avg->iter_ratl_cp);

/*=======================================================================*/
/* III) Construct the Conserved Quantity */

  etot = stat_avg->kinet_cp + stat_avg->cp_ehart + 
            stat_avg->cp_eext + stat_avg->cp_exc + 
            stat_avg->cp_eke + stat_avg->cp_enl;
  if(cp->cptherm_info.num_c_nhc > 0) 
     etot += stat_avg->kinet_nhc_cp 
          + stat_avg->vpotnhc_cp;
  if(simopts_cp == 1) {
     etot += (stat_avg->kinet) + (stat_avg->vintert) 
             +  (stat_avg->vintrat);
     if((ensopts->nvt)==1)  
         {etot     += (stat_avg->kinet_nhc) 
                      +  (stat_avg->vpotnhc);}
    if((ensopts->npt_i)==1){
      etot     += (stat_avg->kinet_nhc) 
                +  (stat_avg->vpotnhc)
                +  (stat_avg->kinet_v)   
                +  (stat_avg->vpot_v);
      if(stat_avg->iswit_vdw==0){etot+=stat_avg->vlong;}
    }
    if((ensopts->npt_f)==1)
         {etot+= (stat_avg->kinet_nhc) 
                 +  (stat_avg->vpotnhc)
                 +  (stat_avg->kinet_v)   
                 +  (stat_avg->vpot_v);
         if(stat_avg->iswit_vdw==0){etot+=stat_avg->vlong;}
	 }/*endif:npt_f*/
  }/* endif  cp on*/
  if((timeinfo->itime)==1)   {stat_avg->econv0 = etot;}
  if((timeinfo->itime)==1 && cp->cpopts.cp_isok_opt == 1) 
                              stat_avg->cp_kconv0 = stat_avg->kinet_cp;

#ifdef DEBUG_CP
  printf("kinet_cp %.12g\n",stat_avg->kinet_cp);
  printf("cp_ehart %.12g\n",stat_avg->cp_ehart);
  printf("cp_eext %.12g\n",stat_avg->cp_eext);
  printf("cp_exc %.12g\n",stat_avg->cp_exc);
  printf("cp_eke %.12g\n",stat_avg->cp_eke);
  printf("cp_enl %.12g\n",stat_avg->cp_enl);
  printf("vpotnhc %.12g\n",stat_avg->vpotnhc_cp);
  printf("kinet_nhc_cp %.12g\n",stat_avg->kinet_nhc_cp);
#endif

/*==================================================================*/
/* IV) Construct Box Stuff */

  getdeth_avg(cell->hmat,&(stat_avg->vol));
  get_cell(cell->hmat,&a,&b,&c,&tab,&tbc,&tac);

/*=======================================================================*/
/* V) Construct Averages */

  stat_avg->econv += fabs((etot-stat_avg->econv0)/stat_avg->econv0);
  if(cp->cpopts.cp_isok_opt == 1)
    stat_avg->cp_kconv += fabs((stat_avg->kinet_cp-stat_avg->cp_kconv0)/stat_avg->cp_kconv0);

  if(simopts_cp==1){
     stat_avg->akinet     += stat_avg->kinet;
     stat_avg->akinet_v   += stat_avg->kinet_v;
     stat_avg->akinet_nhc += stat_avg->kinet_nhc;
     stat_avg->avintert   += stat_avg->vintert;
     stat_avg->avintrat   += stat_avg->vintrat;
     stat_avg->avol       += stat_avg->vol;
     stat_avg->acella     += a;
     stat_avg->acellb     += b;
     stat_avg->acellc     += c;
     stat_avg->acellab    += tab;
     stat_avg->acellbc    += tbc;
     stat_avg->acellac    += tac;
     for ( i = 1;i<=9;i++){
       stat_avg->apten[i] += ((ptens->tvten[i]+ptens->pvten_tot[i])
                        /stat_avg->vol);
     }/*endfor:i*/
     stat_avg->apress += (((ptens->tvten)[1]+ptens->pvten_tot[1]
                       +(ptens->tvten)[5]+(ptens->pvten_tot)[5]
                       +(ptens->tvten)[9]+(ptens->pvten_tot[9]))
                       /(3.0*stat_avg->vol));
  }/*endif*/
  stat_avg->akinet_cp    += stat_avg->kinet_cp;
  stat_avg->akinet_nhc_cp+= stat_avg->kinet_nhc_cp;
  stat_avg->acp_ehart    += stat_avg->cp_ehart;
  stat_avg->acp_eext     += stat_avg->cp_eext;
  stat_avg->acp_exc      += stat_avg->cp_exc;
  stat_avg->acp_eke      += stat_avg->cp_eke;
  stat_avg->acp_enl      += stat_avg->cp_enl;
  if(simopts_cp==1){
     stat_avg->aikinet     += stat_avg->kinet;
     stat_avg->aikinet_v   += stat_avg->kinet_v;
     stat_avg->aikinet_nhc += stat_avg->kinet_nhc;
     stat_avg->aivintert   += stat_avg->vintert;
     stat_avg->aivintrat   += stat_avg->vintrat;
     stat_avg->aivol       += stat_avg->vol;
     stat_avg->aicella     += a;
     stat_avg->aicellb     += b;
     stat_avg->aicellc     += c;
     stat_avg->aicellab    += tab;
     stat_avg->aicellbc    += tbc;
     stat_avg->aicellac    += tac;
     for ( i = 1;i<=9;i++){
       stat_avg->aipten[i] += (ptens->tvten[i]+ptens->pvten_tot[i])
         /stat_avg->vol;
     }/*endfor:i*/
     stat_avg->aipress += (((ptens->tvten)[1]+(ptens->pvten_tot)[1]
                         +(ptens->tvten)[5]+(ptens->pvten_tot)[5]
                         +(ptens->tvten)[9]+(ptens->pvten_tot[9]))
                        /(3.0*stat_avg->vol));
  }/*endif*/
  stat_avg->aikinet_cp    += stat_avg->kinet_cp;
  stat_avg->aikinet_nhc_cp+= stat_avg->kinet_nhc_cp;
  stat_avg->aicp_ehart    += stat_avg->cp_ehart;
  stat_avg->aicp_eext     += stat_avg->cp_eext;
  stat_avg->aicp_exc      += stat_avg->cp_exc;
  stat_avg->aicp_eke      += stat_avg->cp_eke;
  stat_avg->aicp_enl      += stat_avg->cp_enl;

/*======================================================================*/
/*======================================================================*/
}/*endif:myid==0*/
Barrier(world);

/*======================================================================*/
}/*end routine*/
/*==========================================================================*/





