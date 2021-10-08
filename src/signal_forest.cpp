#include "signal_forest.h"

//#define CAN_SWITCH_BUYSELL


SignalForestStrategy::SignalForestStrategy()
{
    m_once = false;
    m_who_entered_idx = -1;
}

SignalForestStrategy::~SignalForestStrategy()
{
    clear_strategies();
}



SignalForestStrategy::SignalForestStrategy( const std::string& a_inp )
{
	std::string t_ap_inp = remove_comments(a_inp);

    // first strip the input from any whitespace
    std::string t_inp;
    for(unsigned int i=0; i<t_ap_inp.size(); i++)
    {
        if (!((t_ap_inp[i] == ' ') ||
            (t_ap_inp[i] == '\n') ||
            (t_ap_inp[i] == '\t') ||
            (t_ap_inp[i] == '\r')))
            t_inp += t_ap_inp[i];
    }

    std::vector< std::string > st;
    split(st, t_inp, is_any_of("$"));

    for(unsigned int i=0; i<st.size(); i++)
    {
        if (st[i] != "")
        {
            SignalTreeStrategy *s = new SignalTreeStrategy(st[i]);
            m_strategies.push_back(s);
        }
    }
    m_once = false; 
    m_who_entered_idx = -1;

    m_max_bars_back = 0;
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        if (m_strategies[i]->m_max_bars_back > m_max_bars_back)
            m_max_bars_back = m_strategies[i]->m_max_bars_back;
    }
}

SignalForestStrategy::SignalForestStrategy( std::vector< SignalTreeStrategy* >& a_strategies )
{
    m_once = false;
    m_who_entered_idx = -1;
    m_strategies = a_strategies;
}

void SignalForestStrategy::add_strategy( SignalTreeStrategy& a_strategy )
{
    m_strategies.push_back(&a_strategy);
}

void SignalForestStrategy::clear_strategies()
{
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        delete m_strategies[i];
    }
    m_strategies.clear();
}

unsigned int SignalForestStrategy::num_strategies()
{
    return m_strategies.size();
}



void SignalForestStrategy::bind( const DataMatrix& data )
{
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        m_strategies[i]->bind(data);
    }
}


Order *SignalForestStrategy::tick(const DataMatrix &data, Env &env, const unsigned int idx)
{
    std::vector<Order*> orders; 
    int t_chosen_order_idx = -1;

    // Tick all strategies and save their returned orders.
    // then we will pick one of them (if any) and delete the others. 
    for(int i=0; i<(int)m_strategies.size(); i++)
    {
        orders.push_back(m_strategies[i]->tick(data, env, idx));
    }

    // loop over all orders to determine which one we choose
    for(int i=0; i<(int)(orders.size()); i++)
    {
        // there is an actual order
        if (orders[i] != NULL)
        {
            // if that's an entry order, use it, 
            // but only if not already in position
            if  ((((orders[i]->order_type == BUY) || (orders[i]->order_type == SELL)) && (env.current_position == NO_POSITION)) 
#ifdef CAN_SWITCH_BUYSELL                
                ||
                ((orders[i]->order_type == SELL) && (env.current_position == LONG_POSITION)) || 
                ((orders[i]->order_type == BUY) && (env.current_position == SHORT_POSITION))
#endif
                )

            {
                t_chosen_order_idx = i;
                m_who_entered_idx = i;

                break;
            }

            // if that's an exit order, use it only if the strategy requesting
            // the order is the same one who entered.
            if (orders[i]->order_type == CLOSE)
            {
                if ((env.current_position != NO_POSITION) && (m_who_entered_idx == i))
                {
                    t_chosen_order_idx = i;
                    m_who_entered_idx = -1;
                }
            }
        }
    }

    // chose anything?
    if (t_chosen_order_idx != -1)
    {
        // delete all orders except the one we chose
        for(int i=0; i<(int)orders.size(); i++)
        {
            if (i != t_chosen_order_idx)
            {
                delete orders[i];
            }
        }

        return orders[t_chosen_order_idx];
    }
    else
    {
        return NULL;
    }
}



std::string SignalForestStrategy::print_EasyLanguage( bool debug/*=false*/, bool readable/*=true*/ )
{
    std::stringstream s;

#if 0
    // initial boilerplate
    s << "using elsystem; \n";

    // global variables
    s << "variable: once_x(true); \n"; 
    s << format(
         "array: qty[%s](0); \n" 
         "array: t_buying[%s](false); \n" 
         "array: t_selling[%s](false); \n" 
         "array: t_closing[%s](false); \n" 
         "array: orders[%s](0); \n" 
         "variable: last_profit(0), last_exit_reason(0), idx(0); \n" 
         "variable: datetime cur_datetime(null); \n"
         "variable: chosen_order_idx(-1); \n"
         "variable: order(-1), tmp(0); \n"
         "variable: who_entered(-1); \n"
         "array: rev_buysell[%s](false); \n"
         ) 
         % num_strategies() % num_strategies() % num_strategies() 
         % num_strategies() % num_strategies() % num_strategies();

    // clear all ELIs
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        m_strategies[i]->m_buy_tree->clear_ELIs();
        m_strategies[i]->m_sell_tree->clear_ELIs();
        m_strategies[i]->m_close_tree->clear_ELIs();
        m_strategies[i]->m_stop_loss_tree->clear_ELIs();
        m_strategies[i]->m_take_profit_tree->clear_ELIs();
        m_strategies[i]->m_amount_per_trade->clear_ELIs();
        m_strategies[i]->m_reversed_buysell->clear_ELIs();
    }

    // output all variable declarations
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        s << m_strategies[i]->m_buy_tree->get_el_var_decl() << 
             m_strategies[i]->m_sell_tree->get_el_var_decl() << 
             m_strategies[i]->m_close_tree->get_el_var_decl() << 
             m_strategies[i]->m_stop_loss_tree->get_el_var_decl() << 
             m_strategies[i]->m_take_profit_tree->get_el_var_decl() <<
             m_strategies[i]->m_amount_per_trade->get_el_var_decl() <<
             m_strategies[i]->m_reversed_buysell->get_el_var_decl();
    }

    // Runtime boilerplate code, any precomputations, etc. 
    s << "cur_datetime = BarDateTime[0]; \n";

    // All Signal init()s
    s << "if once_x then \nbegin \n";
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        s << m_strategies[i]->m_buy_tree->get_el_init_recursive() <<
             m_strategies[i]->m_sell_tree->get_el_init_recursive() <<
             m_strategies[i]->m_close_tree->get_el_init_recursive() <<
             m_strategies[i]->m_amount_per_trade->get_el_init_recursive() <<
             m_strategies[i]->m_reversed_buysell->get_el_init_recursive();
    }
    s << "\nonce_x = false; \nend; \n\n";

    // Update last profit
    s <<"if Marketposition <> 0 then " <<
        "begin " <<
        "   last_profit = PositionProfit; " <<
        "end; ";

    // Tick all stops
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        s << m_strategies[i]->m_stop_loss_tree->get_el_tick_recursive() <<
             m_strategies[i]->m_take_profit_tree->get_el_tick_recursive();
    }

    // clear order variables
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        s << format(
            "t_buying[%s] = false; "
            "t_selling[%s] = false; "
            "t_closing[%s] = false; "
            "orders[%s] = 0; "
            ) % i % i % i % i;
    }

    // Tick all signals and determine orders.
    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        s << m_strategies[i]->m_buy_tree->get_el_tick_recursive() <<
             m_strategies[i]->m_sell_tree->get_el_tick_recursive() <<
             m_strategies[i]->m_close_tree->get_el_tick_recursive() <<
             m_strategies[i]->m_amount_per_trade->get_el_tick_recursive() <<
             m_strategies[i]->m_reversed_buysell->get_el_tick_recursive()
             ;

        for(unsigned int j=0; j<m_strategies.size(); j++)
        {
            s << format(
            		"qty[%s] = round((%s) / close, 0);"
            		"rev_buysell[%s] = %s;"
            )
                % j % m_strategies[j]->m_amount_per_trade->get_el_output()
                % j % m_strategies[j]->m_reversed_buysell->get_el_output();
                ;
        }

        s << "t_buying[" << i <<"] = " << m_strategies[i]->m_buy_tree->get_el_output() << "; \n";
        s << "t_selling[" << i <<"] = " << m_strategies[i]->m_sell_tree->get_el_output() << "; \n";
        s << "t_closing[" << i <<"] = " << m_strategies[i]->m_close_tree->get_el_output() << "; \n";

        // determine order type
        s << format("if t_closing[%s] or %s or %s then "
            "begin "
            "   if %s then orders[%s] = 4 "
            "   else if %s then orders[%s] = 5 "
            "   else orders[%s] = 3; "
            "end "
            "else "
            "begin "
            "   if t_buying[%s] and (not t_selling[%s]) then \n"
            "   begin\n"
            "      orders[%s] = 1;\n"
            "   end;  \n"
            "   if t_selling[%s] and (not t_buying[%s]) then  \n"
            "   begin \n"
            "      orders[%s] = 2;  \n"
            "   end; \n"
            "end;") 
            % i 
            % m_strategies[i]->m_stop_loss_tree->get_el_output()
            % m_strategies[i]->m_take_profit_tree->get_el_output()
            % m_strategies[i]->m_stop_loss_tree->get_el_output() % i
            % m_strategies[i]->m_take_profit_tree->get_el_output() % i
            % i % i % i % i % i % i % i;
    }

    // Choose an order and execute it. 
    s << format(
         "chosen_order_idx = -1; "
         "for idx = 0 to %s "
         "begin "
         "  if orders[idx] <> 0 then "
         "  begin "
         "      if (orders[idx] = 1) or (orders[idx] = 2) then "
         "      begin "
         "          if Marketposition = 0 then " 
         "          begin "
         "              chosen_order_idx = idx;"
         "              who_entered  = idx; "
         "          end; "
         "      end;"
         ""
         "      if (orders[idx] >= 3) then "
         "      begin "
         "          if (Marketposition <> 0) and (who_entered = idx) then "
         "          begin "
         "              chosen_order_idx = idx;"
         "              who_entered = -1;"
         "          end; "
         "      end; "
         "  end;"
         "end;"
         ) % num_strategies();

    // get all sl/tp inits here
    std::string all_sl_buy;
    std::string all_sl_sell;
    std::string all_tp_buy;
    std::string all_tp_sell;

    for(unsigned int i=0; i<m_strategies.size(); i++)
    {
        all_sl_buy += m_strategies[i]->m_stop_loss_tree->get_el_init_buying();
        all_sl_sell += m_strategies[i]->m_stop_loss_tree->get_el_init_selling();
        all_tp_buy += m_strategies[i]->m_take_profit_tree->get_el_init_buying();
        all_tp_sell += m_strategies[i]->m_take_profit_tree->get_el_init_selling();
    }

    // Buying template
    std::stringstream ssbuy;
    ssbuy << format(
        "if rev_buysell[chosen_order_idx] = false then  \n"
        "begin \n"
        "   %s  %s    \n"
        "   buy qty[chosen_order_idx] shares this bar; \n"
        "end         \n"
        "else        \n"
        "begin       \n"
        "   %s  %s    \n"
        "   sellshort qty[chosen_order_idx] shares this bar; \n"
        "end; \n") 
        % all_sl_buy  % all_tp_buy
        % all_sl_sell % all_tp_sell;

    // Selling template
    std::stringstream sssell;
    sssell << format(
        "if rev_buysell[chosen_order_idx] = false then  \n"
        "begin \n"
        "   %s  %s    \n"
        "   sellshort qty[chosen_order_idx] shares this bar; \n"
        "end         \n"
        "else        \n"
        "begin       \n"
        "   %s  %s    \n"
        "   buy qty[chosen_order_idx] shares this bar; \n"
        "end; \n")
        % all_sl_sell % all_tp_sell
        % all_sl_buy  % all_tp_buy;

    std::string sclose = "sell this bar; buytocover this bar; \n";
    std::string sclose_sl = "sell (\"Stop Loss LX\") this bar; buytocover (\"Stop Loss SX\") this bar; \n";
    std::string sclose_tp = "sell (\"Take Profit LX\") this bar; buytocover (\"Take Profit SX\") this bar; \n";

    // add to master string stream
    s << format(
        "order = 0; \n"
        "if (chosen_order_idx > -1) then "
        "order = orders[chosen_order_idx]; \n"

        "if (order>=3) then  \n"
        "begin \n"
        "   if (order=4) then " 
        "   begin "
        "      last_exit_reason = -1;"
        "      %s "
        "   end "
        "   else if (order=5) then "
        "   begin "
        "      last_exit_reason = 1;"
        "      %s "
        "   end "
        "   else "
        "   begin "
        "      last_exit_reason = 0;"
        "      %s;  \n"
        "   end; "
        "end \n"
        "else \n"
        "begin \n"
        "   if (order = 1) then \n"
        "   begin\n"
        "      %s;\n"
        "   end;  \n"
        "   if (order = 2) then  \n"
        "   begin \n"
        "      %s;  \n"
        "   end; \n"
        "end; \n")
        % sclose_sl
        % sclose_tp
        % sclose 
        % ssbuy.str() 
        % sssell.str();
#endif

    return s.str();
}

std::string SignalForestStrategy::print_MQL4( bool debug/*=false*/, bool readable/*=true*/ )
{
    return "TODO";
}

std::string make_forest_EL( std::string a_expr )
{
    SignalForestStrategy st(a_expr);
    return st.print_EasyLanguage();
}

std::string make_forest_MQL4( std::string a_expr )
{
    SignalForestStrategy st(a_expr);
    return st.print_MQL4();
}
