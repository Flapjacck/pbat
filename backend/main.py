#!/usr/bin/env python3
"""
Pokemon Battle Pipeline
Main file that orchestrates Pokemon selection and battles
"""

import sys
import os

# Since main.py is now in the backend directory, we can import directly
from pokemon_selector import PokemonSelector
from pokemon_battle import Pokemon, BattleSimulator

class PokemonBattlePipeline:
    def __init__(self):
        self.selector = PokemonSelector()
        self.battle_simulator = BattleSimulator()
        self.selected_pokemon = []
    
    def display_welcome(self):
        """Display welcome message and instructions"""
        print("Welcome to Pokemon Battle Arena!")
        print("Choose your Pokemon and start battling!")
    
    def test_api_connection(self) -> bool:
        """Test connection to PokeAPI before starting"""
        print("\n🔍 Testing API connection...")
        if not self.selector.test_connection():
            print("\n❌ Cannot connect to PokeAPI!")
            print("Please check your internet connection and try again.")
            return False
        print("✅ API connection successful!")
        return True
    
    def select_pokemon(self, player_num: int) -> Pokemon:
        """Guide user through Pokemon selection process"""
        print(f"\n{'🎯 PLAYER ' + str(player_num) + ' POKEMON SELECTION'}")
        print("=" * 50)
        
        # Get Pokemon name
        while True:
            pokemon_name = input(f"Player {player_num}, enter your Pokemon name: ").strip()
            if not pokemon_name:
                print("Please enter a Pokemon name.")
                continue
            
            print(f"🔍 Fetching data for {pokemon_name}...")
            pokemon_data = self.selector.get_pokemon_data(pokemon_name)
            
            if pokemon_data:
                print(f"✅ Found {pokemon_data['name'].capitalize()}!")
                break
            else:
                print("❌ Pokemon not found. Please try again.")
        
        # Get available moves
        available_moves = self.selector.get_pokemon_moves(pokemon_data)
        print(f"📋 Found {len(available_moves)} available moves")
        
        # Get level
        level = self.get_pokemon_level()
        
        # Get IVs
        print(f"\n⚡ IV SELECTION FOR {pokemon_data['name'].upper()}")
        print("-" * 40)
        ivs = self.get_pokemon_ivs()
        
        # Get EVs
        print(f"\n💪 EV SELECTION FOR {pokemon_data['name'].upper()}")
        print("-" * 40)
        evs = self.get_pokemon_evs()
        
        # Get Moves
        print(f"\n🎯 MOVE SELECTION FOR {pokemon_data['name'].upper()}")
        print("-" * 40)
        moves = self.get_pokemon_moves(available_moves)
        
        # Get Held Item
        print(f"\n📦 HELD ITEM SELECTION FOR {pokemon_data['name'].upper()}")
        print("-" * 40)
        held_item = self.get_held_item()
        
        # Create Pokemon object
        pokemon = Pokemon(
            name=pokemon_data['name'],
            pokemon_data=pokemon_data,
            ivs=ivs,
            evs=evs,
            moves=moves,
            level=level,
            held_item=held_item
        )
        
        # Display final summary
        self.display_pokemon_summary(pokemon, player_num)
        
        return pokemon
    
    def get_pokemon_level(self) -> int:
        """Get Pokemon level from user"""
        while True:
            try:
                level_input = input("Enter Pokemon level (1-100, default 50): ").strip()
                if not level_input:
                    return 50
                
                level = int(level_input)
                if 1 <= level <= 100:
                    return level
                else:
                    print("Level must be between 1 and 100.")
            except ValueError:
                print("Please enter a valid number.")
    
    def get_pokemon_ivs(self) -> dict:
        """Get IVs with enhanced options"""
        print("Choose IV selection method:")
        print("r. Random IVs")
        print("2. Perfect IVs (all 31)")
        print("3. Manual entry")
        print("4. Competitive preset (31 in important stats)")
        
        while True:
            choice = input("Enter choice (r, 2-4): ").strip().lower()
            
            if choice == "r":
                ivs = self.selector.generate_random_ivs()
                print("🎲 Random IVs generated!")
                self.display_ivs(ivs)
                return ivs
            
            elif choice == "2":
                ivs = {stat: 31 for stat in self.selector.stats}
                print("⭐ Perfect IVs set!")
                self.display_ivs(ivs)
                return ivs
            
            elif choice == "3":
                return self.selector.get_user_ivs()
            
            elif choice == "4":
                # Competitive preset: 31 in important offensive stats, 0 in unused attack stat
                ivs = {stat: 31 for stat in self.selector.stats}
                # Ask which attack stat to minimize
                print("Competitive preset: Which attack stat should be minimized?")
                print("1. Physical Attack (for special attackers)")
                print("2. Special Attack (for physical attackers)")
                attack_choice = input("Enter choice (1-2): ").strip()
                
                if attack_choice == "1":
                    ivs["attack"] = 0
                    print("🏆 Competitive IVs set (Special Attacker build)")
                elif attack_choice == "2":
                    ivs["special-attack"] = 0
                    print("🏆 Competitive IVs set (Physical Attacker build)")
                else:
                    print("🏆 Competitive IVs set (Balanced build)")
                
                self.display_ivs(ivs)
                return ivs
            
            else:
                print("Please enter r, 2, 3, or 4.")
    
    def get_pokemon_evs(self) -> dict:
        """Get EVs with enhanced options"""
        print("Choose EV selection method:")
        print("r. Random EVs")
        print("2. No EVs (all 0)")
        print("3. Manual entry")
        print("4. Competitive presets")
        
        while True:
            choice = input("Enter choice (r, 2-4): ").strip().lower()
            
            if choice == "r":
                evs = self.selector.generate_random_evs()
                print("🎲 Random EVs generated!")
                self.display_evs(evs)
                return evs
            
            elif choice == "2":
                evs = {stat: 0 for stat in self.selector.stats}
                print("📊 No EVs set!")
                return evs
            
            elif choice == "3":
                return self.selector.get_user_evs()
            
            elif choice == "4":
                return self.get_competitive_ev_preset()
            
            else:
                print("Please enter r, 2, 3, or 4.")
    
    def get_competitive_ev_preset(self) -> dict:
        """Get competitive EV presets"""
        print("\nCompetitive EV Presets:")
        print("1. Physical Sweeper (252 Atk, 252 Spe, 4 HP)")
        print("2. Special Sweeper (252 SpA, 252 Spe, 4 HP)")
        print("3. Physical Tank (252 HP, 252 Def, 4 Atk)")
        print("4. Special Tank (252 HP, 252 SpD, 4 SpA)")
        print("5. Balanced (85 in each stat)")
        
        while True:
            preset = input("Choose preset (1-5): ").strip()
            
            if preset == "1":
                evs = {"hp": 4, "attack": 252, "defense": 0, "special-attack": 0, "special-defense": 0, "speed": 252}
                print("⚔️ Physical Sweeper EVs set!")
                break
            elif preset == "2":
                evs = {"hp": 4, "attack": 0, "defense": 0, "special-attack": 252, "special-defense": 0, "speed": 252}
                print("🔮 Special Sweeper EVs set!")
                break
            elif preset == "3":
                evs = {"hp": 252, "attack": 4, "defense": 252, "special-attack": 0, "special-defense": 0, "speed": 0}
                print("🛡️ Physical Tank EVs set!")
                break
            elif preset == "4":
                evs = {"hp": 252, "attack": 0, "defense": 0, "special-attack": 4, "special-defense": 252, "speed": 0}
                print("🔰 Special Tank EVs set!")
                break
            elif preset == "5":
                evs = {stat: 85 for stat in self.selector.stats}
                print("⚖️ Balanced EVs set!")
                break
            else:
                print("Please enter 1, 2, 3, 4, or 5.")
        
        self.display_evs(evs)
        return evs
    
    def get_pokemon_moves(self, available_moves: list) -> list:
        """Get moves with enhanced options"""
        print(f"Available moves: {len(available_moves)} total")
        print("First 10 moves:", ", ".join(available_moves[:10]))
        if len(available_moves) > 10:
            print("... and more")
        
        print("\nMove selection options:")
        print("r. Random 4 moves")
        print("2. Manual selection")
        print("3. Show all moves and select")
        
        while True:
            choice = input("Enter choice (r, 2-3): ").strip().lower()
            
            if choice == "r":
                moves = self.selector.get_user_moves(available_moves)
                # Override to make it random
                import random
                moves = random.sample(available_moves, min(4, len(available_moves)))
                print("🎲 Random moves selected:")
                for i, move in enumerate(moves, 1):
                    print(f"  {i}. {move.replace('-', ' ').title()}")
                return moves
            
            elif choice == "2":
                return self.selector.get_user_moves(available_moves)
            
            elif choice == "3":
                print(f"\nAll {len(available_moves)} available moves:")
                for i, move in enumerate(available_moves, 1):
                    print(f"{i:3d}. {move.replace('-', ' ').title()}")
                    if i % 20 == 0:  # Pause every 20 moves
                        input("Press Enter to continue...")
                return self.selector.get_user_moves(available_moves)
            
            else:
                print("Please enter r, 2, or 3.")
    
    def display_ivs(self, ivs: dict):
        """Display IV summary"""
        print("IVs:", end=" ")
        for stat, iv in ivs.items():
            print(f"{stat.capitalize()}: {iv}", end=", ")
        print()
    
    def display_evs(self, evs: dict):
        """Display EV summary"""
        print("EVs:", end=" ")
        total = sum(evs.values())
        for stat, ev in evs.items():
            if ev > 0:
                print(f"{stat.capitalize()}: {ev}", end=", ")
        print(f"Total: {total}/510")
    
    def display_pokemon_summary(self, pokemon: Pokemon, player_num: int):
        """Display detailed Pokemon summary"""
        print(f"\n{'🌟 PLAYER ' + str(player_num) + ' POKEMON READY'}")
        print("=" * 50)
        print(f"Name: {pokemon.name}")
        print(f"Level: {pokemon.level}")
        print(f"Types: {', '.join(pokemon.types).title()}")
        print(f"HP: {pokemon.max_hp}")
        
        if pokemon.held_item:
            print(f"Held Item: {pokemon.held_item.replace('-', ' ').title()}")
        
        print("\nFinal Stats:")
        for stat_name, value in pokemon.stats.items():
            print(f"  {stat_name.replace('-', ' ').title()}: {value}")
        
        print("\nMoves:")
        for i, move in enumerate(pokemon.moves, 1):
            print(f"  {i}. {move.replace('-', ' ').title()}")
        
        print("=" * 50)
    
    def battle_menu(self, pokemon1: Pokemon, pokemon2: Pokemon):
        """Display battle options menu"""
        print(f"\n🥊 BATTLE SETUP: {pokemon1.name} vs {pokemon2.name}")
        print("=" * 50)
        print("Battle Options:")
        print("1. Manual Battle (you choose each move)")
        print("2. Auto Battle (AI chooses moves)")
        print("3. View Pokemon Stats")
        print("4. Return to main menu")
        
        while True:
            choice = input("Enter choice (1-4): ").strip()
            
            if choice == "1":
                print("\n🎮 Starting Manual Battle!")
                input("Press Enter to begin...")
                self.battle_simulator.simulate_battle(pokemon1, pokemon2, auto_battle=False)
                break
            
            elif choice == "2":
                print("\n🤖 Starting Auto Battle!")
                input("Press Enter to begin...")
                self.battle_simulator.simulate_battle(pokemon1, pokemon2, auto_battle=True)
                break
            
            elif choice == "3":
                self.compare_pokemon_stats(pokemon1, pokemon2)
            
            elif choice == "4":
                return
            
            else:
                print("Please enter 1, 2, 3, or 4.")
    
    def compare_pokemon_stats(self, pokemon1: Pokemon, pokemon2: Pokemon):
        """Compare stats of two Pokemon"""
        print(f"\n📊 POKEMON COMPARISON")
        print("=" * 70)
        print(f"{'Stat':<15} {pokemon1.name:<20} {pokemon2.name:<20} {'Advantage':<15}")
        print("-" * 70)
        
        for stat_name in pokemon1.stats.keys():
            stat1 = pokemon1.stats[stat_name]
            stat2 = pokemon2.stats[stat_name]
            
            if stat1 > stat2:
                advantage = pokemon1.name
            elif stat2 > stat1:
                advantage = pokemon2.name
            else:
                advantage = "Tie"
            
            print(f"{stat_name.replace('-', ' ').title():<15} {stat1:<20} {stat2:<20} {advantage:<15}")
        
        print("-" * 70)
        print(f"Types: {', '.join(pokemon1.types).title():<25} {', '.join(pokemon2.types).title():<25}")
        print("=" * 70)
        input("\nPress Enter to continue...")
    
    def main_menu(self):
        """Display main menu and handle user choices"""
        while True:
            print(f"\n{'🏠 MAIN MENU'}")
            print("=" * 30)
            print("1. Quick Battle (2 random Pokemon)")
            print("2. Custom Battle (select both Pokemon)")
            print("3. Exit")
            
            choice = input("Enter choice (1-3): ").strip()
            
            if choice == "1":
                self.quick_battle()
            
            elif choice == "2":
                self.custom_battle()
            
            elif choice == "3":
                print("\n👋 Thanks for using Pokemon Battle Arena!")
                print("May your battles be legendary!")
                break
            
            else:
                print("Please enter 1, 2, or 3.")
    
    def quick_battle(self):
        """Quick battle with simplified setup"""
        print("\n⚡ QUICK BATTLE MODE")
        print("=" * 30)
        print("Enter two Pokemon names for a quick battle!")
        print("(Random IVs/EVs and moves will be assigned)")
        
        # Get Pokemon names only
        pokemon_names = []
        for i in range(2):
            while True:
                name = input(f"Enter Pokemon {i+1} name: ").strip()
                if name:
                    pokemon_names.append(name)
                    break
                print("Please enter a Pokemon name.")
        
        # Create Pokemon with random stats
        pokemon_list = []
        for i, name in enumerate(pokemon_names):
            print(f"\n🔍 Setting up {name}...")
            pokemon_data = self.selector.get_pokemon_data(name)
            
            if not pokemon_data:
                print(f"❌ Could not find {name}. Skipping quick battle.")
                return
            
            available_moves = self.selector.get_pokemon_moves(pokemon_data)
            
            import random
            pokemon = Pokemon(
                name=pokemon_data['name'],
                pokemon_data=pokemon_data,
                ivs=self.selector.generate_random_ivs(),
                evs=self.selector.generate_random_evs(),
                moves=random.sample(available_moves, min(4, len(available_moves))),
                level=50,
                held_item=random.choice([None, 'leftovers', 'life-orb', 'choice-band', 'focus-sash', 'expert-belt'])
            )
            pokemon_list.append(pokemon)
            print(f"✅ {pokemon.name} ready!")
        
        # Start battle
        self.battle_simulator.simulate_battle(pokemon_list[0], pokemon_list[1], auto_battle=True)
        input("\nPress Enter to continue...")
    
    def custom_battle(self):
        """Full custom battle with detailed Pokemon selection"""
        print("\n⚔️ CUSTOM BATTLE MODE")
        print("=" * 30)
        
        # Select both Pokemon
        pokemon1 = self.select_pokemon(1)
        pokemon2 = self.select_pokemon(2)
        
        # Battle menu
        self.battle_menu(pokemon1, pokemon2)
    
    def single_pokemon_builder(self):
        """Build a single Pokemon and view its stats"""
        print("\n🔧 POKEMON BUILDER")
        print("=" * 30)
        
        pokemon = self.select_pokemon(1)
        print(f"\n✅ {pokemon.name} has been created successfully!")
        
        # Show detailed breakdown
        print(f"\nDetailed Stats Breakdown:")
        print(f"Level {pokemon.level} {pokemon.name}")
        print(f"Types: {', '.join(pokemon.types).title()}")
        print(f"Base Stats vs Final Stats:")
        
        for stat_info in pokemon.pokemon_data['stats']:
            stat_name = stat_info['stat']['name']
            base_stat = stat_info['base_stat']
            final_stat = pokemon.stats[stat_name]
            iv = pokemon.ivs[stat_name]
            ev = pokemon.evs[stat_name]
            
            print(f"  {stat_name.replace('-', ' ').title()}: {base_stat} → {final_stat} (IV: {iv}, EV: {ev})")
        
        input("\nPress Enter to continue...")
    
    def get_held_item(self) -> str:
        """Get held item selection from user"""
        print("Choose held item option:")
        print("1. No held item")
        print("2. Competitive items (Choice Band, Life Orb, etc.)")
        print("3. Utility items (Leftovers, Focus Sash, etc.)")
        print("4. Random item")
        print("5. Manual entry")
        
        while True:
            choice = input("Enter choice (1-5): ").strip()
            
            if choice == "1":
                return None
            
            elif choice == "2":
                return self.get_competitive_item()
            
            elif choice == "3":
                return self.get_utility_item()
            
            elif choice == "4":
                items = ['leftovers', 'life-orb', 'choice-band', 'choice-specs', 'choice-scarf',
                        'focus-sash', 'assault-vest', 'expert-belt', 'muscle-band', 'wise-glasses']
                import random
                item = random.choice(items)
                print(f"🎲 Random item selected: {item.replace('-', ' ').title()}")
                return item
            
            elif choice == "5":
                item_name = input("Enter held item name (or 'none' for no item): ").strip().lower()
                if item_name in ['', 'none', 'no', 'nothing']:
                    return None
                return item_name.replace(' ', '-')
            
            else:
                print("Please enter 1, 2, 3, 4, or 5.")
    
    def get_competitive_item(self) -> str:
        """Get competitive held item selection"""
        print("\nCompetitive Items:")
        print("1. Choice Band (+50% Attack, locked into one move)")
        print("2. Choice Specs (+50% Sp. Attack, locked into one move)")
        print("3. Choice Scarf (+50% Speed, locked into one move)")
        print("4. Life Orb (+30% damage, 10% recoil)")
        print("5. Assault Vest (+50% Sp. Defense, no status moves)")
        print("6. Expert Belt (+20% super effective damage)")
        
        items = {
            '1': 'choice-band', '2': 'choice-specs', '3': 'choice-scarf',
            '4': 'life-orb', '5': 'assault-vest', '6': 'expert-belt'
        }
        
        while True:
            choice = input("Choose item (1-6): ").strip()
            if choice in items:
                item = items[choice]
                print(f"🏆 {item.replace('-', ' ').title()} selected!")
                return item
            print("Please enter 1, 2, 3, 4, 5, or 6.")
    
    def get_utility_item(self) -> str:
        """Get utility held item selection"""
        print("\nUtility Items:")
        print("1. Leftovers (heals 1/16 HP each turn)")
        print("2. Focus Sash (survives KO at full HP)")
        print("3. Focus Band (10% chance to survive any KO)")
        print("4. Quick Claw (20% chance to move first)")
        print("5. Muscle Band (+10% physical damage)")
        print("6. Wise Glasses (+10% special damage)")
        print("7. Flame Orb (burns holder)")
        print("8. Toxic Orb (poisons holder)")
        
        items = {
            '1': 'leftovers', '2': 'focus-sash', '3': 'focus-band',
            '4': 'quick-claw', '5': 'muscle-band', '6': 'wise-glasses',
            '7': 'flame-orb', '8': 'toxic-orb'
        }
        
        while True:
            choice = input("Choose item (1-8): ").strip()
            if choice in items:
                item = items[choice]
                print(f"🔧 {item.replace('-', ' ').title()} selected!")
                return item
            print("Please enter 1, 2, 3, 4, 5, 6, 7, or 8.")
    
    def run(self):
        """Main application entry point"""
        self.display_welcome()
        
        # Test API connection
        if not self.test_api_connection():
            return
        
        # Start main menu
        self.main_menu()

if __name__ == "__main__":
    app = PokemonBattlePipeline()
    app.run()
